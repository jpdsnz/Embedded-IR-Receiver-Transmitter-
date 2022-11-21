// John Jones
// ID: 1001639122
// Lab 9
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "uart0.h"
#include "cterminal.h"
#include "speaker.h"
#include "eeprom.h"
//#include "irled.h"

//---- DEBUG DEFINES ---- //
//#define REMOTEP //Remote Print buttons
#define LAB7 //Let lab 7 run
#define LAB9

//-----------------------------------------------------------------------------
// Bitband Aliases
//-----------------------------------------------------------------------------

#define gpOut        (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 0*4))) //Port b
#define gpInput      (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4))) //Port b
#define infra_LED    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 6*4))) //Port b

//-----------------------------------------------------------------------------
// Port Masks
//-----------------------------------------------------------------------------
// Port B Masks
#define gpO_Mask 1
#define gpI_Mask 32
#define infra_LED_Mask 64


// Other defines
#define data_Val 48

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// ---- IR Receive ---- //
uint8_t buffer[data_Val];
bool faultChk = false;
char NL = '\0';
uint8_t sample_location = 0;
uint8_t smp_Nine = 2; //9ms Value
uint8_t smp_Four = 5; //4ms Value
uint8_t smp_Mid = 53; //Address Value
uint8_t smp_Max = 101; //End Value
uint8_t remAdd = 0; //Address for remote control

// ---- Cmd ---- //
uint8_t sT = 0; // Used for reusable indexing
uint16_t eprmAdd = 0; //2Bytes
bool cmdLearn = false;
char cmdBuff[12];
extern bool spKalertG;
extern bool spKalertB;

// ---- IR Timings ---- //
//uint32_t deltaT [] = {154000, 60000, 72385, 22798};
uint32_t deltaT [] = {154000, 60000, 72385, 22798};
uint8_t sampleTest[] = {0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1};


// ---- IR LED ---- //
uint32_t deltaT2[] = {360000, 180000, 22798}; //Timing changed slightly
uint8_t typedAdd = 0, iDx= 0, typedData= 0, bT_Val= 0, cNt2= 0, tempVal= 0, everyBit = 0; //Declare Variables
bool finishedBit = true;
uint8_t fullBit = 8;

//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------

void printGpI()
{
  putcUart0(gpInput + data_Val); //Print out the binary output
}

void playCmd(uint8_t addr, uint8_t data)
{
    typedAdd = addr;
    typedData = data;
    //Enable the PWM timer to shoot IR LED out
    TIMER2_IMR_R = TIMER_IMR_TATOIM;
    TIMER2_CTL_R |= TIMER_CTL_TAEN;
}

void ledTISR(void)
{
    TIMER2_ICR_R = TIMER_ICR_TATOCINT; // Clear interrupt
    if(iDx >= (smp_Max-1) )
    {
        TIMER2_IMR_R &= ~TIMER_IMR_TATOIM; // turn off timer interrupt
        TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // turn off timer
        GPIO_PORTB_DEN_R &= ~infra_LED_Mask; // turn off IR LED
        typedAdd = typedData = iDx = bT_Val = cNt2 = everyBit = 0; //Init Vars
        finishedBit = true;
        return;
    }

    if (iDx == 0)
      GPIO_PORTB_DEN_R |= infra_LED_Mask; //Flash the IR LED

    else if (iDx == 1)
      GPIO_PORTB_DEN_R &= ~infra_LED_Mask; //Turn off IR LED

    else if( iDx >= smp_Nine && iDx <= (smp_Max-2) )
    {
        if( iDx == smp_Nine )
        {
            tempVal = typedAdd;
        }

        else if( iDx == (smp_Mid-3) )
        {
            tempVal = typedData;
        }

        else if( iDx == (smp_Max-3) )
        {
            tempVal = 0;
        }

        if ( finishedBit )
        {
           if(tempVal & 128>>cNt2)
               bT_Val = 1;
           else
               bT_Val = 0;

           finishedBit = false;
        }

        if (bT_Val == 0)
        {
           if(everyBit % 2 == 0)
               GPIO_PORTB_DEN_R |= infra_LED_Mask;
           else
               GPIO_PORTB_DEN_R &= ~infra_LED_Mask;
           everyBit++;
        }

        else if (bT_Val == 1)
        {
           if(everyBit % 4 == 0)
               GPIO_PORTB_DEN_R |= infra_LED_Mask;
           else
               GPIO_PORTB_DEN_R &= ~infra_LED_Mask;
           everyBit++;
        }

        if( (everyBit == 2 && bT_Val == 0) || (everyBit == 4 && bT_Val == 1) )
        {
           finishedBit = true;
           cNt2++;
           everyBit = 0;
        }

        if (cNt2 == fullBit)
        {
            cNt2 = 0;
            tempVal = ~tempVal;
            finishedBit = true;
        }
    }

    if(iDx <= 2)
    {
       TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
       TIMER2_TAILR_R = deltaT2[iDx];
       TIMER2_CTL_R |= TIMER_CTL_TAEN;
    }
    iDx++;
}


void printAdd(uint8_t num)
{
    int lastDigit = num % 10;
    num /= 10;
    int secondDigit = num % 10;
    num /= 10;
    int firstDigit = num % 10;
    putcUart0(48 + firstDigit);
    putcUart0(48 + secondDigit);
    putcUart0(48 + lastDigit);
}

void prntZero(uint8_t num)
{
    int firstDigit = num % 10;
    putcUart0(48+firstDigit);
}

void printValues(uint8_t a, uint8_t d)
{
   putsUart0("\n\rAddress:    ");
   prntZero(a);
   //putsUart0(" |");
   putsUart0("\n\rData Value: ");
   printAdd(d);
   putsUart0("\n\r");
}




void retBtnData(uint8_t ir_Data)
{
   printValues(remAdd, ir_Data);

    #ifdef REMOTEP
       putsUart0("\n\rBtn: ");
    #endif
   switch (ir_Data)
   {
    case 162:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("ch-");
        #endif
        break;
    case 98:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("ch");
        #endif
        break;
    case 226:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("ch+");
        #endif
        break;
    case 34:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("prev");
        #endif
        break;
    case 2:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("next");
        #endif
        break;
    case 194:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("play/pause");
        #endif
        break;
    case 224:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("-");
        #endif
        break;
    case 168:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("+");
        #endif
        break;
    case 144:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("eq");
        #endif
        break;
    case 104:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("0");
        #endif
        break;
    case 152:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("100+");
        #endif
        break;
    case 176:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("200+");
        #endif
        break;
    case 48:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("1");
        #endif
        break;
    case 24:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("2");
        #endif
        break;
    case 122:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("3");
        #endif
        break;
    case 16:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("4");
        #endif
        break;
    case 56:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("5");
        #endif
        break;
    case 90:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("6");
        #endif
        break;
    case 66:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("7");
        #endif
        break;
    case 74:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("8");
        #endif
        break;
    case 82:
            spkHappy();
        #ifdef REMOTEP
            putsUart0("9");
        #endif
        break;
    default:
        //spKalertB = false; //testing alert
        //if(spKalertB)
        //{
            spkSad();
            putsUart0("Bad Command!\n\r");
        //}
        #ifdef REMOTEP
            putsUart0("Wrong Button!\r");
        #endif
        break;
       }
   putsUart0("\n\r");
}

void fillBuffer()
{
    uint8_t i, ir_Data, countBit; //Declare vars
    int cT = 0;
    i = ir_Data = countBit = 0; //Init vars
    // Use Data buffer and fill in values where necessary
    for(i =0; i < data_Val; i++)
    {
        // Checks for 0 and 1
        if (buffer[i] == 0)
            cT++;
        else if(buffer[i] == 1)
            cT--;
        if(cT == 0)
        {
            ir_Data <<= 1; //shift bits rightward
            countBit++;
        }
        else if(cT == -2) //Found 3 ones in a row
        {
            cT = 0;
            ir_Data |= 1; //Add one to byte
        }
        if(countBit == fullBit) //byte fully constructed
        {
            retBtnData(ir_Data); //Get button value from data

            // ---- Learning Command Code ----//

            //if Learning is enabled then write the data found here into the eeprom
            if(cmdLearn)
            {
                //char buffer[] = "play1";
                uint32_t eprmCMD = 0; //4 bytes This will be sent to eeprom
                 //Address (eprmAdd) 2 bytes will also be sent to eeprom
                int index = 0;
                while(eprmAdd < 3) //This stores the actual string command in the eeprom
                {
                    for(index = 0; index < 4; index++)
                    {
                        char c = cmdBuff[index];
                        eprmCMD |= c;
                        eprmCMD <<= 8; //Shift over 8 bits (1byte)
                    }
                    writeEeprom(eprmAdd, eprmCMD);
                    eprmAdd++;
                }
                eprmCMD |= remAdd; // Add address into the command storage in the eeprom
                eprmCMD <<= 8; // Shift over the last 8 bits
                eprmCMD |= ir_Data;
                writeEeprom(eprmAdd, eprmCMD);
                eprmAdd++;
                cmdLearn = false;
                putsUart0("Finished learning...\n\r");
            }
            // ---- End Learning Command ---- //
            return;
        }
    }
}

void timerISR(void)
{
    TIMER1_ICR_R = TIMER_ICR_TATOCINT; // Clear interrupt
    if ( sample_location == (smp_Max+1) || faultChk)
    {
        fillBuffer(); //Fill buffer with values obtained
        TIMER1_IMR_R &= ~TIMER_IMR_TATOIM; // Clear interrupt flag
        TIMER1_CTL_R &= ~TIMER_CTL_TAEN; // Timer 1 turn off
        GPIO_PORTB_IM_R |= gpI_Mask; // GPI interrupt turn on (ONLY WANT TO RUN DURING COMMANDS
        return;
    }
    gpOut ^= 1; //Toggle GPO
    //printGpI(); //Print out values for debugging
    if (sample_location <= (52) && gpInput^sampleTest[sample_location]) //XOR is 1 when both are different
    {
      faultChk = true; //Error Found in comparison
      //TIMER1_IMR_R &= ~TIMER_IMR_TATOIM; // Clear interrupt flag
      //TIMER1_CTL_R &= ~TIMER_CTL_TAEN; // Timer 1 turn off
    }
    else if(sample_location >= smp_Mid && sample_location <= (100)) //If you are at the data portion
        buffer[sample_location-smp_Mid] = gpInput;  //Place data into buffer
    // Change load value for timer here
    if(sample_location >= 2 && sample_location <= 5)
    {
        TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
        TIMER1_TAV_R = 0;
        TIMER1_TAILR_R = deltaT[sample_location - 2]; //Minus 2 because of my timer array is only 4
        TIMER1_CTL_R |= TIMER_CTL_TAEN;
    }
    sample_location++; //Move the sample location up
}


// Initialize Infrared LED
void initIR()
{
    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0; //Note UART on port A must use AP
    // Enable timers and clocks
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    _delay_cycles(3);
    //Infrared LED Masks
    GPIO_PORTB_DIR_R |= infra_LED_Mask;                       // make bit5 an output
    GPIO_PORTB_DR2R_R |= infra_LED_Mask;                      // set drive strength to 2mA
    GPIO_PORTB_DEN_R &= ~infra_LED_Mask;                      // Keep infra_LED off
    GPIO_PORTB_AFSEL_R |= infra_LED_Mask;                     // select auxiliary function
    GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB6_M;                    // enable PWM
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB6_M0PWM0;                // M0PWM0 on PB6

    // Configure PWM module 0 to drive RGB backlight
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                         // init PWM0
    SYSCTL_SRPWM_R = 0;                                       // clear SRPWM reg
    // turn off PWM0 generator 1
    PWM0_0_CTL_R = 0;

    PWM0_0_GENA_R = PWM_0_GENA_ACTCMPBD_ZERO | PWM_0_GENA_ACTLOAD_ONE; // output 3 on PWM0, gen 1b, cmpb
    PWM0_0_LOAD_R = 1047;                                     // Period = to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
    PWM0_INVERT_R = PWM_INVERT_PWM0INV;                       // Outputs are inverted so that duty cyc incr w/ incr compare vals
    PWM0_0_CMPB_R = 523;                                      // infra_LED off (0=always low, 1023=always high)

    // Turn on PWM
    PWM0_0_CTL_R = PWM_0_CTL_ENABLE;                          // PWM gen 0 turn on
    PWM0_ENABLE_R = PWM_ENABLE_PWM0EN;                        // Enable output for PWM0

    //Initialize Timer 2
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
    TIMER2_TAILR_R = 0;
    TIMER2_IMR_R &= ~TIMER_IMR_TATOIM;
    NVIC_EN0_R |= 1 << (INT_TIMER2A-16);
}

void feISR(void) //Change in startup
{
    GPIO_PORTB_ICR_R = GPIO_ICR_GPIO_M; // Clear the interrupts
    GPIO_PORTB_IM_R &= ~gpI_Mask; // Disable interrupt
    TIMER1_TAILR_R = 90000; //First timer value of 2.25ms loaded in
    TIMER1_IMR_R |= TIMER_IMR_TATOIM; //Turn on interrupt
    TIMER1_CTL_R |= TIMER_CTL_TAEN; //Turn on timer

   /*
    ***FOR REFERENCE***
    * // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 1 Hz interrupt rate
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    NVIC_EN0_R |= 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    */

   // Reset the variables
   sample_location = 0;
   faultChk = false;
   // Init the buffer
   for(sample_location = 0; sample_location < data_Val; sample_location++)
       buffer[sample_location] = 0;
   sample_location = 0; //Default sample location for sampling bits

}




// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable timers and clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1; //Port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;   //Port B
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;     //Port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;   //Port E
    _delay_cycles(3);

    // Configure GPI
    GPIO_PORTB_DEN_R |= gpI_Mask;
    GPIO_PORTB_DIR_R |= ~gpI_Mask;

    // Gpio output
    GPIO_PORTB_DIR_R |= gpO_Mask;
    GPIO_PORTB_DR2R_R |= gpO_Mask;
    GPIO_PORTB_DEN_R |= gpO_Mask;

    // Falling edge GPIO
    GPIO_PORTB_IS_R &= ~gpI_Mask;
    GPIO_PORTB_IBE_R &= ~gpI_Mask;
    GPIO_PORTB_IEV_R &= ~ gpI_Mask;
    GPIO_PORTB_ICR_R |= gpI_Mask;
    NVIC_EN0_R |= 1 << (INT_GPIOB-16); // Configure the interrupts for port b

    //Initialize Timer 1
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
    NVIC_EN0_R |= 1 << (INT_TIMER1A-16);

    //GPIO_PORTB_IM_R |= gpI_Mask; // Enable Port B interrupts (ONLY WANT TO RUN DURING COMMANDS
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initSpk();
    initIR();
    initEeprom();
    initUart0();
    setUart0BaudRate(115200,40e6);
    putsUart0("Lab 9\n\r");

    while( true )
    {
        putsUart0("Enter a command:\n\r");
        USER_DATA data;
        data.fieldCount = 0;
        getsUart0(&data);
        parseFields(&data);

        //#ifdef LAB9
        //// ---- Part 1 Decode ---- ////
        if( isCommand(&data, "decode", 0) ) //type decode
        {
            putsUart0("Now decoding...\n\r");
            putsUart0("Press a button on remote!\n\r");
            //Enable IR interrupt to take in data and address from remote
            GPIO_PORTB_IM_R |= gpI_Mask;
        }

        //// ---- Part 2 Learn ---- ////
        else if( isCommand(&data, "learn", 1) ) //type learn "commandname"
        {
            sT = 0;
            char *getCMD = getFieldString(&data, 1);
            // Copy string to buffer
            for(sT = 0; sT < getCMD[sT] != NL ; sT++)
                cmdBuff[sT] = getCMD[sT];
            cmdBuff[sT] = NL;
            cmdLearn = true;
            putsUart0("Now Learning...\n\r");
            //Enable IR interrupt to take in data and address from remote
            GPIO_PORTB_IM_R |= gpI_Mask;
        }


        //// ---- Part 7 Alert ---- ////
        else if( isCommand(&data, "alert", 2) )
        {

            char *choice = getFieldString(&data, 1);
            char *sw = getFieldString(&data, 2);
            // Turning on Good alerts
            if(cmP(choice,"good"))
            {
                if( cmP(sw,"on") )
                {
                    spKalertG = true; // Default is true
                    putsUart0("Turning on good alert!\n\r");
                }
                else if( cmP(sw,"off") )
                {
                    putsUart0("Turning off good alert!\n\r");
                    spKalertG = false; // Default is true
                }

            }
            // Turning on Bad Alerts
            else if( cmP(choice,"bad") )
            {
                if( cmP(sw,"on") )
                {
                    putsUart0("Turning on bad alert!\n\r");
                    spKalertB = true;
                }
                else if( cmP(sw,"off") )
                {
                    putsUart0("Turning off bad alert!\n\r");
                    spKalertB = false;
                }
            }
        }

        /*

        else if( isCommand(&data, "alert bad on", 2) )
        {
            putsUart0("Turning on bad alert!\n\r");
            spKalertB = true; // Default is true
        }

        else if( isCommand(&data, "alert good off", 0) )
        {
            putsUart0("Turning off good alert!\n\r");
            spKalertG = false;
        }

        else if( isCommand(&data, "alert bad off", 0) )
        {
            putsUart0("Turning off bad alert!\n\r");
            spKalertB = false;
        }
        #endif
        */

        #ifdef LAB7
                else if( isCommand(&data, "play", 2) ) // type play 0 162
                {
                    int32_t typedAdd = getFieldInteger(&data, 1);
                    int32_t typedData = getFieldInteger(&data, 2);
                    playCmd(findInt(&data, typedAdd), findInt(&data, typedData));
                }
        #endif
    }
}
