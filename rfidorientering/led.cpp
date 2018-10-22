#include "led.h"
#include "config.h"
#include "arduino.h"
  
void flashLed(int onDelay, int offDelay){
  digitalWrite(LED1_PIN, LED_ON);
  delay(onDelay);             
  digitalWrite(LED1_PIN, LED_OFF); 
  delay(offDelay);  
}

void flashLedReverse(int delayBeforeOn, int onTime){
  delay(delayBeforeOn);  
  digitalWrite(LED1_PIN, LED_ON);
  delay(onTime);             
  digitalWrite(LED1_PIN, LED_OFF); 
}
