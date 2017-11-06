/* 
 * File:   HR_FUNCTIONS.h
 * Author: nicholasmitchell
 *
 * Created on November 4, 2017, 2:48 AM
 */

#ifndef HR_FUNCTIONS_H
#define	HR_FUNCTIONS_H
#include<p18f452.h>

void DelayFor18TCY( void );
void DelayPORXLCD (void);
void DelayXLCD (void);
void config_TMRS(void);
void config_LCD(void);
void config_PINS(void);
void start_up(void);
int BPM(void);
void display(char bpm_out[3]);
#endif	/* HR_FUNCTIONS_H */

