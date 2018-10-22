#ifndef _CONFIG_H
#define _CONFIG_H

// CONFIG
extern const int PIEZO_PIN;
extern const int LED1_PIN;
extern const int LED2_PIN;
extern const int BUTTON_PIN;
extern const int SPI_SS_PIN;

// make it possible to reverse led functionality without changing the code
extern const int LED_ON;
extern const int LED_OFF;

extern const int BUTTON_ON;
extern const int BUTTON_OFF;

extern const unsigned short int WATCH_ID;

// Delays
extern const int DELAY_AFTER_START;
extern const int DELAY_AFTER_NOT_RUNNING;
extern const int DELAY_AFTER_END_RUN;
extern const int DELAY_AFTER_UNKNOWN;
extern const int DELAY_AFTER_TOO_SOON_REPEAT;
extern const int DELAY_AFTER_RESULTS_SENT;

#endif
