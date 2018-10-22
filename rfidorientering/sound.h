#ifndef _SOUND_H
#define _SOUND_H

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

extern void play(int freq, short toneLength, short toneSpacing);

#endif
