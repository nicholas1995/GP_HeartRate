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

    int count_int=0;


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

//    if(PIR2bits.TMR3IF==1)              //TMR3 Overflow
//    {
        //PORTCbits.RC2=0;
        INTCONbits.GIE=0;
        PIR1bits.TMR1IF=0;    //clear interrupt flag bit for timer3
    
        WriteTimer1(52992);
        count_int++;
        PORTCbits.RC2=!PORTCbits.RC2;
        INTCONbits.GIE=1;
//    }

}

/*
 **********************************************************************************************************
 *                                  CONFIGERATION
 **********************************************************************************************************
 */
void config_TMRS(void)
{      
    OpenTimer1(TIMER_INT_ON &
             T1_16BIT_RW &
             T1_SOURCE_INT &
             T1_PS_1_8 &
            `T1_OSC1EN_OFF   &
             T1_SYNC_EXT_OFF);
    WriteTimer1(52992);
    
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
    TRISCbits.RC2=0;
    PORTCbits.RC2=0;

}
void start_up(void)
{
    //This must be run at the start of the program to initialise everything
    config_TMRS();
    config_LCD();
    config_PINS();
}

/*
 **********************************************************************************************************
 *                                          MAIN
 **********************************************************************************************************
 */
void main(void)
{
    int count_char[5];
    start_up();

    INTCONbits.GIE=1;
    while(1)
    {
        itoa(count_int,count_char);
            while(BusyXLCD());
            SetDDRamAddr( 0x00 );
            putsXLCD(count_char);
        
    }
}
