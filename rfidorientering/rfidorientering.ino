//#define ADAFRUIT

#include <Wire.h>
#ifdef ADAFRUIT
  #include <Adafruit_PN532.h>
  int PN532_IRQ = 7;
  int PN532_RESET = 6;  
  Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
#else
  #include <PN532_I2C.h>
  #include <PN532.h>
  #include <NfcAdapter.h>
  PN532_I2C intf(Wire);
  PN532 nfc = PN532(intf);
#endif


const int D_4 = 261;
const int E_4 = 293;
const int F_4 = 349;
const int G_4 = 392;
const int G_4_SHARP = 415;
const int A_4 = 440;
const int C_5 = 523;
const int E_5 = 659;
const int G_5 = 784;
const int A_5 = 880;
const int C_6 = 1046;

const short len16th = 1;
const short len8th = 2;
const short lenQuarter = 4;
const short lenHalf = 8;

// CONFIG
int PIEZO_PIN = 9;
int LED_PIN = 8;


const int postCount = 4;
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

int getPostIndex(long id) {
  for(int i=0; i<postCount; i++){
    if(posts[i] == id){
      return i;
    }
  }
  return postCount;
}

void flashLed(int onDelay, int offDelay){
  digitalWrite(LED_PIN, HIGH);
  delay(onDelay);             
  digitalWrite(LED_PIN, LOW); 
  delay(offDelay);  
}

void play(int freq, short toneLength, short toneSpacing) {
  short toneOn = 75 * toneLength;
  short toneOff = 75 * toneSpacing - toneOn;
  tone(PIEZO_PIN, freq, toneOn); 
  flashLed(toneOn, toneOff);
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
    play(A_5, len8th, lenQuarter); 
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
  Serial.print(" (");    
  Serial.print(id, DEC);    
  Serial.println(")"); 
}

void startRun() {
  Serial.println("Klar, ferdig, gå! Ha et morsomt løp!");
  isRunning = true;
  indicateStart();

  for(int i=0; i<postCount; i++){
    checkins[i] = false;
    checkintimes[i] = 0;    
  }
  startTime = millis();
  delay(3000);
}

void checkIn(long id){

  if(!isRunning){
    indicateNotRunning();
    delay(3000);
    return;  
  }
  
  int index = getPostIndex(id);
  
  if(index == postCount){
    indicateUnknown();
    Serial.print("Ukjent id");
    printId(id);
    delay(3000);
    return;
  }

  // Wait 3 seconds between each possible checkin, and flash a different signal
  // without updating checkin status/time if post has already been checked.
  if(checkins[index] == true) {
    if(lastCheckinTime < millis() - 3000){ 
      Serial.print("Sjekket inn tidligere");     
      printId(id); 
      indicateDuplicate();
    } else {
      Serial.print("For kort tid siden sist");
      printId(id);
      delay(2000);
      return;
    }
  } else {
    checkins[index] = true;
    checkintimes[index] = millis() - startTime;  
    Serial.print("Sjekker inn post ");
    Serial.print(index + 1, DEC);    
    printId(id);
    indicateSuccess();    
  }
  
  lastCheckinTime = millis();
}

void printTime(long aTime){
  int totalSeconds = aTime / 1000;
  int secondsPart = totalSeconds % 60;
  int minutesPart = totalSeconds / 60;
  Serial.print(minutesPart, DEC);
  Serial.print(" min ");
  Serial.print(secondsPart, DEC);
  Serial.print(" sek");
}

void getResults() {
  Serial.println("Resultat:");
  Serial.print("  Total tid: ");
  printTime(endTime - startTime);
  Serial.println();
  Serial.println();
  
  Serial.println("Passeringstider, poster:");
  bool success = true;
  for(int i=0; i<postCount; i++){ 
    Serial.print("  ");
    Serial.print(i+1, DEC);   
    if(checkins[i] == true){    
      Serial.print(": Funnet etter ");
      printTime(checkintimes[i]);  
      Serial.println();
    } else {
      Serial.println(": Ikke funnet");
      success = false;
    }
  }

  Serial.println();

  if(success == true){
    Serial.println("Gratulerer, du fant alle postene!");
  } else {
    Serial.println("Du fant ikke alle postene, men godt jobbet likevel!");
  }
}

void endRun() {
  if(isRunning){
    endTime = millis();
    isRunning = false;
  }
  Serial.println("Gratulerer, du er i mål!"); 
  Serial.println(""); 
  getResults(); 
  indicateEnd();
  delay(3000);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIEZO_PIN, OUTPUT);

  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(9600);
  Serial.println("Starter orienteringsklokke");
  #ifdef ADAFRUIT
    Serial.println("Bruker Adafruit-bibliotek");
  #else
    Serial.println("Bruker Seeed-Studio-bibliotek");
  #endif

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Kunne ikke finne PN53x-kortet");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Fant PN5-chip"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Venter på ISO14443A-kort...");  
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {    
    // Display some basic information about the card
    /*Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");*/

    // Get LSBs of chip as a long.
    // UID may be 4 or 7 bytes long so we need to know the length to take the last bytes. We chop off the
    // 24 MSBs if this is a 8 bytes long address, but even then it would be extremely surprising if we get an
    // address collision.
    unsigned long longId = 0;
    longId += (long) uid[uidLength-4] << 24;
    longId += (long) uid[uidLength-3] << 16;
    longId += (long) uid[uidLength-2] << 8;
    longId += uid[uidLength-1];

    if(longId == startId){
      startRun();
    } else if(longId == endId){
      endRun();
    } else {
      checkIn(longId);   
    }
  }
 
}
