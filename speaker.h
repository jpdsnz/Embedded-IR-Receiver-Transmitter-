#ifndef SPEAKER_H
#define SPEAKER_H


#define sPk          (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4))) //Port e

#define SPEAKERSTART //For debugging
//#define SPEAKTST

// Port E Masks
#define sPk_Mask 16

void spKLoad(uint32_t f, uint32_t time0, uint32_t time1, uint32_t time2 );
void spkSad();
void spkHappy();
void startUpSpkr();
void initSpk();







#endif /* TERMINAL_H */
