#include <p18f452.h>
#include "HR_FUNCTIONS.h"
#include<delays.h>
#include"xlcd.h"
#include<timers.h>

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

int count_RB=0;             //This counts the amount of time RB6 sees a rising edge 
int count_PREVENT=0;         //This is used to block readings within 300ms of each other
int allow_reading=1;
int program_start=1;
int count_RB_previous=0;
int BpM=0; 
/*
 ***********************************************************************************************************
 *                                       FUNCTIONS
 ***********************************************************************************************************  
 */
/*
 *******************************************************************************
 *                                  CONFIGERATION
 *******************************************************************************
 */
/*
 ***********************************************************
 *                        TMR3 
 ***********************************************************
 */
void config_TMRS(void)
{      
    OpenTimer3(TIMER_INT_ON &
             T3_16BIT_RW &
             T3_SOURCE_INT &
             T0_PS_1_8 &
             T3_SYNC_EXT_OFF);
    WriteTimer3(52992);
    RCONbits.IPEN = 1;              //Enable Priority Levels
}
/*
 ***********************************************************
 *                        XLCD
 ***********************************************************
 */
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
/*
 ***********************************************************
 *                        PINS 
 ***********************************************************
 */
void config_PINS(void)
{
    //CONFIGURE PORT B TO USE INTERRUPT ON CHANGE
    TRISC=0x00;
    TRISBbits.RB0=1;
    PORTC=0x00;     //Set all portC pins as low outputs 
    INTCONbits.INT0IE=1;//INT0 External Interrupt Enable bit
    INTCON2bits.INTEDG0=1;//INTERRUPT ON RISING EDGE
}
/*
 **************************************************************************************
 *                         FUNCTION CALLS AT START 
 **************************************************************************************
 */
void start_up(void)
{
    //This must be run at the start of the program to initialise everything
    config_TMRS();
    config_LCD();
    config_PINS();
}
/*
 **************************************************************************************
 *                         CALCULATE BPM USING BEATS IN 5 SECONDS
 **************************************************************************************
 */


int BPM(void)
{   
    if(program_start==1)
    {
        BpM = count_RB*12;
        count_RB_previous=BpM;
    }
    if(program_start==0)
    {
        count_RB=count_RB*12;
        count_RB_previous=((count_RB_previous+count_RB)/2);
        BpM=count_RB_previous;
    }
            return BpM;

}
/*
 **************************************************************************************
 *                              DISPLAY READING 
 **************************************************************************************
 */
void display(char bpm_out[3])
{
    while(BusyXLCD());
    WriteCmdXLCD(0b00000001);

    while(BusyXLCD());
    SetDDRamAddr( 0x00 );
    putrsXLCD( "HR(bps):");

    while(BusyXLCD());
    SetDDRamAddr( 0x08 );
    putsXLCD(bpm_out);
    
}
