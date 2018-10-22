#include "arduino.h"
#include "sound.h"
#include "led.h"
#include "config.h"

void play(int freq, short toneLength, short toneSpacing) {
  short toneOn = 75 * toneLength;
  short toneOff = 75 * toneSpacing - toneOn;
  tone(PIEZO_PIN, freq, toneOn); 
  flashLedReverse(toneOn, toneOff);
}
