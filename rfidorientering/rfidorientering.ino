#include "emulatetag.h"
#include "NdefMessage.h"
#include "debug.h"
#include "sound.h"
#include "led.h"
#include "config.h"

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532Interface.h>
#include <PN532.h>

const String RESULTS_URL = "vg.no/r?r=";

// The following must be added to the library header PN532/emulatetag.h, under public:
//   PN532 getPn532();
PN532 EmulateTag::getPn532(){
   return pn532;
}

PN532_SPI interface(SPI, SPI_SS_PIN);
EmulateTag tagEmulator(interface);
PN532 reader = tagEmulator.getPn532();

uint8_t uid[3] = { 0x12, 0x34, 0x56 };
uint8_t ndefBuf[120];
NdefMessage message;
int messageSize;

const unsigned int MODE_READER = 1;
const unsigned int MODE_TAG = 2;
unsigned int mode = MODE_READER;
const short postCount = 4;

unsigned long posts[] = {
  2380092371,
  2429244883,
  884273261,
  2313901011
};

bool checkins[postCount];
unsigned long checkintimes[postCount];

unsigned long startId = 1434717671;
unsigned long endId = 1963724773;

boolean isRunning = false;
unsigned long startTime;
unsigned long endTime;
unsigned long lastCheckinTime = -1;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIEZO_PIN, OUTPUT);

  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif

  #ifdef DEBUG
    Serial.begin(115200);
  #endif

  // uid must be 3 bytes!
  tagEmulator.setUid(uid);  
  tagEmulator.init();

  uint32_t versiondata = reader.getFirmwareVersion();
  if (! versiondata) {
    DMSG("Kunne ikke finne PN53x-kortet");
    while (1); // halt
  }
  // Got ok data, print it out!
  DMSG("Fant PN5-chip"); DMSG_HEX((versiondata>>24) & 0xFF); 
  DMSG("Firmware ver. "); DMSG((versiondata>>16) & 0xFF, DEC); 
  DMSG('.'); DMSG_INT((versiondata>>8) & 0xFF);
  
  // configure board to read RFID tags  
  
  DMSG_STR("Venter på ISO14443A-kort...");  
}

int getPostIndex(long id) {
  for(int i=0; i<postCount; i++){
    if(posts[i] == id){
      return i;
    }
  }
  return postCount;
}

void indicateStart() {
  play(C_5, len16th, lenQuarter);
  play(C_5, len16th, len8th);
  play(C_5, len16th, len8th);
  play(G_5, lenQuarter, lenHalf);
}

void indicateEnd() {
  play(C_5, len16th, len8th);
  play(C_5, len16th, len8th);
  play(C_5, len16th, lenQuarter);
  play(E_5, len16th, lenQuarter);
  play(G_5, len16th, lenQuarter);
  play(C_6, lenQuarter, lenHalf);
}

void indicateNotRunning() {
  play(C_5, len8th, lenQuarter);
  play(C_5, len8th, lenQuarter);
  play(C_5, len8th, lenQuarter);
  play(G_4_SHARP, lenHalf, lenHalf);  
}

void indicateSuccess() { 
  play(C_5, len16th, len8th);  
  play(G_5, len8th, lenQuarter);
  play(C_5, len16th, len8th);
  play(G_5, lenQuarter, lenHalf);
}

void indicateDuplicate() {
  for(byte i = 0; i < 4; i++){
    play(C_5, len16th, lenQuarter); 
  }
}

void indicateUnknown() {
  play(A_4, len16th, len16th); 
  play(G_4, len16th, len16th); 
  play(F_4, len16th, len16th); 
  play(E_4, len16th, len16th); 
  play(D_4, len16th, len16th); 
}

void printId(unsigned long id){
  DMSG(" (");    
  DMSG(id, DEC);    
  DMSG_STR(")"); 
}

void startRun() {
  DMSG_STR("Klar, ferdig, gå! Ha et morsomt løp!");
  isRunning = true;
  indicateStart();

  for(int i=0; i<postCount; i++){
    checkins[i] = false;
    checkintimes[i] = 0;    
  }
  startTime = millis();
  delay(DELAY_AFTER_START);
}

void checkIn(long id){

  if(!isRunning){
    indicateNotRunning();
    delay(DELAY_AFTER_NOT_RUNNING);
    return;  
  }
  
  int index = getPostIndex(id);
  
  if(index == postCount){
    indicateUnknown();
    DMSG("Ukjent id");
    printId(id);
    delay(DELAY_AFTER_UNKNOWN);
    return;
  }

  // Wait 3 seconds between each possible checkin, and flash a different signal
  // without updating checkin status/time if post has already been checked.
  if(checkins[index] == true) {
    if(lastCheckinTime < millis() - 3000){ 
      DMSG("Sjekket inn tidligere");     
      printId(id); 
      indicateDuplicate();
    } else {
      DMSG("For kort tid siden sist");
      printId(id);
      delay(DELAY_AFTER_TOO_SOON_REPEAT);
      return;
    }
  } else {
    checkins[index] = true;
    checkintimes[index] = millis() - startTime;  

    DMSG("Sjekker inn post ");
    DMSG(index + 1, DEC);    
    printId(id);
    indicateSuccess();    
  }
  
  lastCheckinTime = millis();
}

void printTime(long aTime){
  int totalSeconds = aTime / 1000;
  int secondsPart = totalSeconds % 60;
  int minutesPart = totalSeconds / 60;
  DMSG(minutesPart, DEC);
  DMSG(" min ");
  DMSG(secondsPart, DEC);
  DMSG(" sek");
}

void getResults() {
  DMSG_STR("Resultat:");
  DMSG("  Total tid: ");
  printTime(endTime - startTime);
  DMSG_STR();
  DMSG_STR();
  
  DMSG_STR("Passeringstider, poster:");
  bool success = true;
  for(int i=0; i<postCount; i++){ 
    DMSG("  ");
    DMSG(i+1, DEC);   
    if(checkins[i] == true){    
      DMSG(": Funnet etter ");
      printTime(checkintimes[i]);  
      DMSG_STR();
    } else {
      DMSG_STR(": Ikke funnet");
      success = false;
    }
  }

  DMSG_STR();

  if(success == true){
    DMSG_STR("Gratulerer, du fant alle postene!");
  } else {
    DMSG_STR("Du fant ikke alle postene, men godt jobbet likevel!");
  }
}

String getAsPaddedHex(int number){
  if(number < 16){
    return String("000" + String(number, HEX));
  } else if(number < 256){
    return String("00" + String(number, HEX));
  } else if(number < 8192){
    return String("0" + String(number, HEX));
  } else {
    return String(number, HEX);
  }
}

String getResultsString() { 
  String result = String(WATCH_ID, HEX);

  // get results in tenths.
  for(unsigned short i=0; i<postCount; i++){
    result = String(result + getAsPaddedHex(checkintimes[i] / 100));
  }
  return result;
}

void endRun() {
  if(isRunning){
    endTime = millis();
    isRunning = false;
  }
  DMSG_STR("Gratulerer, du er i mål!"); 
  DMSG_STR(""); 
  #ifdef DEBUG
    getResults(); 
    DMSG_STR(getResultsString());
  #endif
  indicateEnd();
  delay(DELAY_AFTER_END_RUN);
}

void sendResultsToMobile() {
  // start card emulation to let mobile read results
  DMSG_STR("------- Send results --------");
  
  flashLed(100, 100);
  flashLed(100, 100);
  flashLed(100, 100);  
  
  message = NdefMessage();
  message.addUriRecord(String(RESULTS_URL + getResultsString()));
  messageSize = message.getEncodedSize();
  if (messageSize > sizeof(ndefBuf)) {
      DMSG_STR("ndefBuf is too small");
      while (1) { }
  }
  message.encode(ndefBuf);
  tagEmulator.setNdefFile(ndefBuf, messageSize); 

  digitalWrite(LED1_PIN, LED_ON);
  tagEmulator.emulate(10000);
  DMSG("Send time ended");
  digitalWrite(LED1_PIN, LED_OFF); 
}

unsigned long getTagUidAsLong(uint8_t uid[], uint8_t uidLength) {
  // Get LSBs of chip as a long.
  // UID may be 4 or 7 bytes long so we need to know the length to take the last bytes. We chop off the
  // 24 MSBs if this is a 8 bytes long address, but even then it would be extremely surprising if we get an
  // address collision.
  unsigned long longId = 0;
  longId += (long) uid[uidLength-4] << 24;
  longId += (long) uid[uidLength-3] << 16;
  longId += (long) uid[uidLength-2] << 8;
  longId += uid[uidLength-1];
  return longId;
}

void scanForTag(){
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = reader.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
  
  if (success) {    
    unsigned long longId = getTagUidAsLong(uid, uidLength);
    if(longId == startId){
      startRun();
    } else if(longId == endId){
      endRun();             
    } else {
      checkIn(longId);   
    }
  }
}

void loop(void) {
  if (BUTTON_ON == digitalRead(BUTTON_PIN)){
    mode = MODE_TAG;
  }

  if(mode == MODE_READER) {    
    scanForTag();
  } else if(mode == MODE_TAG) {
    sendResultsToMobile();
    mode = MODE_READER;
    delay(DELAY_AFTER_RESULTS_SENT); 
  }
 
}
