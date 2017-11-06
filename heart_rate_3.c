#include<p18f452.h>
#include<timers.h>
#include<delays.h>
#include"xlcd.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

#pragma config  OSC=HS
#pragma config  LVP=OFF
#pragma config  WDT=OFF

void DelayFor18TCY( void )
{

    Delay1TCY();
    Delay1TCY();
    Delay1TCY();
    Delay1TCY();
    Delay10TCYx(1);
}

void DelayPORXLCD (void)
{
 Delay10KTCYx(6);
}

void DelayXLCD (void)
{
 Delay10KTCYx(2); 
}
/*What does this suppose to do 
 * 1. Everytime the users high pulses it breakes the infra red light casuing a peak at the output
 * 2. This peak has a max of about 2V after the front end
 * 3. To register a high the pin has to have a voltage greater than 0.25Vdd-0.8Vdd (TTL logic pins)
 * 4. We will be using TMR1 in counter mood, thus anytime TOCKI(RA4) registers a high it increments the counter
 * 5. We will use TMR3 to operate as a timer that will overflow at .1s.
 * 6. When this overflows it will increment a counter variable that will count to 100.
 * 7. When this reaches 100 we will read the value in TMR1 using ReadTimer1 and this will be the 
 *    the amount of beats in 10 seconds
 *  8. We will then multiply this value by 6 to give the heart beat in beats per minuite 
 * 
 */
#define ALLOW   1
#define BLOCK   0
#define RESTART 0

void low_isr(void);
void high_isr(void);
/*
 **********************************************************************************************************
 *                                  GLOBAL VARIABLES
 **********************************************************************************************************
 */
int count_TMR1=0;           //This keeps a track of the amount of time TMR3 overflows 
int count_RB=0;             //This counts the amount of time RB6 sees a rising edge 
int count_PREVENT=0;         //This is used to block readings within 300ms of each other
int allow_reading=1;

/*
 **********************************************************************************************************
 *                               LOW PRIORITY INTERRUPT
 **********************************************************************************************************
 */
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
    _asm
    goto low_isr
    _endasm
}
#pragma code        //return to the default section of code

#pragma interruptlow low_isr
void low_isr(void)
{ 

    
}

/*
 **********************************************************************************************************
 *                               HIGH PRIORITY INTERRUPT
 **********************************************************************************************************
 */
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
    _asm
    goto high_isr
    _endasm
}
#pragma code

#pragma interrupt high_isr
void high_isr(void)
{
    INTCONbits.GIE=0;
    if(INTCONbits.INT0IF==1)              //RB0 changed states.
    {
        if(allow_reading==ALLOW)
        {
            count_RB++;
            PORTCbits.RC0=!PORTCbits.RC0; //anytime the input switches from low to high RB7 will switch states
            allow_reading=BLOCK;
        }   
        INTCONbits.INT0IF=0;
    }
    if(PIR2bits.TMR3IF==1)              //TMR3 Overflow
    {
        PIR2bits.TMR3IF=0;    //clear interrupt flag bit for timer3
        WriteTimer3(15616);
        count_TMR1++;
        count_PREVENT++;
    }
    INTCONbits.GIE=1;
}

/*
 **********************************************************************************************************
 *                                  CONFIGERATION
 **********************************************************************************************************
 */
void config_TMRS(void)
{      
    OpenTimer3(TIMER_INT_ON &
             T3_16BIT_RW &
             T3_SOURCE_INT &
             T0_PS_1_8 &
             T3_SYNC_EXT_OFF);
    WriteTimer3(15616);
    
    RCONbits.IPEN = 1;              //Enable Priority Levels
}
void config_LCD(void)
{
    OpenXLCD( FOUR_BIT & LINES_5X7 );
    while( BusyXLCD() );
    WriteCmdXLCD( FOUR_BIT & LINES_5X7 );
    while( BusyXLCD() );
    WriteCmdXLCD( BLINK_ON );
    while( BusyXLCD() );
    WriteCmdXLCD( SHIFT_DISP_LEFT );
}
void config_PINS(void)
{
    //CONFIGURE PORT B TO USE INTERRUPT ON CHANGE
    //TRISB=0x5F;
    TRISBbits.RB0=1;
    TRISCbits.RC0=0;
    //TRISBbits.RB7=0;//Set port B pin 7 as input to use to identify pulses(interrupt) and <4-0> will be used for the input for the keypad
    PORTC=0x00;     //Set all portB pins as low outputs 
    INTCONbits.INT0IE=1;//INT0 External Interrupt Enable bit
    INTCONbits.RBIE=1; //INTCON<3> enables this 
    INTCON2bits.INTEDG0=1;//INTERRUPT ON RISING EDGE
}
void start_up(void)
{
    //This must be run at the start of the program to initialise everything
    config_TMRS();
    config_LCD();
    config_PINS();
}
int BPM()
{
    int BPM=0;
    BPM = count_RB*12;
    return BPM;
}
/*
 **********************************************************************************************************
 *                                          MAIN
 **********************************************************************************************************
 */
void main(void)
{
    int beats_5s;
    int current=0;
    char bpm_out[3];
    int bpm=0;
    start_up();

    INTCONbits.GIE=1;
    while(1)
    {
        if(count_PREVENT==3)
        {
            allow_reading=ALLOW;
            count_PREVENT=RESTART;
        }
        
        if(count_TMR1==10)//SHOULD BE 50
        {      

            bpm=BPM();
            itoa(bpm,bpm_out);

            while(BusyXLCD());
            WriteCmdXLCD(0b00000001);


            while(BusyXLCD());
            SetDDRamAddr( 0x00 );
            putrsXLCD( "Heart Rate(BPM):");
            
            while(BusyXLCD());
            SetDDRamAddr( 0x40 );
            putsXLCD(bpm_out);

            count_TMR1=RESTART;
            count_RB=0;
            //current=0;
        }
        
    }
}
