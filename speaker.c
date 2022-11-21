#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "wait.h"
//#include "uart0.h"
//#include "terminal.h"
#include "speaker.h"

bool spKalertG = true; //Represent an alert for good command
bool spKalertB = true; //Represent an alert for bad command
// Play startup sound
void startUpSpkr()
{
    //First Frequency from initiaized value
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask; //Turn on
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask; //Turn off

    //Change Frequency
    PWM1_1_LOAD_R = 15000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (15000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

    //Change Frequency
    PWM1_1_LOAD_R = 10000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (10000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

    //Change Frequency
    PWM1_1_LOAD_R = 15000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (15000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;


    //Change Frequency
    PWM1_1_LOAD_R = 8000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (8000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

    //Change Frequency
    PWM1_1_LOAD_R = 15000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (15000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

    //Change Frequency
    PWM1_1_LOAD_R = 10000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (10000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

    //Change Frequency
    PWM1_1_LOAD_R = 15000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (15000 / 2) - 1;    //Freq at 50 percent duty cycle
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R |= sPk_Mask;
    waitMicrosecond(75000);
    GPIO_PORTE_DEN_R &= ~sPk_Mask;

}

// Change the speaker frequency
void spKLoad(uint32_t f, uint32_t time0, uint32_t time1, uint32_t time2 )
{
        GPIO_PORTE_DEN_R &= ~sPk_Mask;
        waitMicrosecond(time0);
        PWM1_1_LOAD_R = f;
        PWM1_INVERT_R = PWM_INVERT_PWM2INV;
        PWM1_1_CMPB_R = (f / 2) - 1;    //Freq at 50 percent duty cycle
        waitMicrosecond(time1);
        GPIO_PORTE_DEN_R |= sPk_Mask;
        waitMicrosecond(time2);
        GPIO_PORTE_DEN_R &= ~sPk_Mask;
}

/*
// play sound from speaker
void playMusic()
{
    waitMicrosecond(1e6);
    GPIO_PORTE_DEN_R |= SPEAKER_MASK;
    waitMicrosecond(1e6);
    GPIO_PORTE_DEN_R &= ~SPEAKER_MASK;
}
*/

void spkSad()
{
    //GPIO_PORTE_DEN_R &= ~sPk_Mask;
    if(spKalertB == true)
    {
        spKLoad(30000,1000,120000,1e6);
        spKLoad(40000,1000,120000,1e6);
    }
    //spKLoad(75000,100000,120000,120000);
}

void spkHappy()
{
    if(spKalertG == true)
    {
        spKLoad(15000,1e3,170000,1e5);
        spKLoad(12000,1e3,120000,1e6);
    }
}

// Initialize Speaker
void initSpk()
{
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;   //Port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; //Port E
    _delay_cycles(3);
    SYSCTL_GPIOHBCTL_R = 0;          //Set GPIO to use APB

    // Speaker Masks
    GPIO_PORTE_DIR_R |= sPk_Mask;   // make bit 4 an output
    GPIO_PORTE_DR2R_R |= sPk_Mask;  // set drive strength to 2mA
    GPIO_PORTE_DEN_R |= sPk_Mask;   // enable digital
    GPIO_PORTE_AFSEL_R |= sPk_Mask;

    GPIO_PORTE_PCTL_R &= GPIO_PCTL_PE4_M;
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M1PWM2;

    //Config PWMM1_1 PE4 M1 PWM 1a
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;
    SYSCTL_SRPWM_R = 0;
    PWM1_1_CTL_R = 0;                   //Turn of Gen 1 before enabling
    PWM1_1_GENA_R |= PWM_1_GENA_ACTCMPBD_ZERO | PWM_1_GENA_ACTLOAD_ONE;
    PWM1_1_LOAD_R = 30000;              //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (30000 / 2) - 1;    //Freq at 50 percent duty cycle

    // Enable the functionality of the speaker
    PWM1_1_CTL_R = PWM_1_CTL_ENABLE;
    PWM1_ENABLE_R = PWM_ENABLE_PWM2EN;
    GPIO_PORTE_DEN_R &= ~sPk_Mask;
    // Startup sound played through the speaker
    #ifdef SPEAKERSTART
        startUpSpkr();
    #endif

    #ifdef SPEAKTST
            spkSad();
            spkHappy();
    #endif
}
