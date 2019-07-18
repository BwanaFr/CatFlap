/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File: peripherials.h
 * Author: mdonze
 * Comments: I/O utilities
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PERI_INCLUDED_H
#define	PERI_INCLUDED_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>       /* For true/false definition */
#include <stdint.h>

//RFID demodulated stream input
#define RFID_STREAM PORTAbits.RA2
//Red lock solenoid
#define RED_LOCK PORTAbits.RA5
//L293D 3/4 outputs enable
#define RFID_RL_ENABLE PORTCbits.RC0
//Buzzer
#define BUZZER PORTCbits.RC1
//RFID excitation frequency. should not be used
#define RFID_EXCT PORTCbits.RC2
//L293D logic power. Maybe use for battery usage
#define L293_LOGIC PORTCbits.RC3
//Green lock solenoid
#define GREEN_LOCK PORTCbits.RC4
//I/O on serial header. Not used
#define SER_INT PORTCbits.RC5
//Door switch. Internal pull-up required
#define DOOR_SWITCH PORTBbits.RB0
//Lock common output. Direction inversion?
#define COMMON_LOCK PORTBbits.RB1
//L293D 1/2 outputs enable
#define CL_GL_ENABLE PORTBbits.RB2
//LM324 op-amp power. Used when on battery
#define LM324_PWR PORTBbits.RB3
//Red LED
#define RED_LED PORTBbits.RB4
//Green ELD
#define GREEN_LED PORTBbits.RB5
//Red button. Internal pull-up required
#define RED_BTN PORTBbits.RB6
//Green button. Internal pull-up required
#define GREEN_BTN PORTBbits.RB7
//RFID field frequency
#define RFID_FREQ 134200

// Timer 1 is configured with a 1:4 scaler
// Given the CLK freq of 19,600,000 Hz, Fosc/4 = 4,900,000Hz
//(Fosc/4)/Prescaler = 1,225,000 Hz
// 1E-3/((1/(Fosc/4)/Prescaler)-1 = 1224
// 0xFFFF - 1225 = 64331 -> 0xFB37
//Timer 1 high bits preset
#define TMR1_H_PRES 0xFB
//Timer 1 low bits preset
#define TMR1_L_PRES 0x37

/**
 * Initialize peripherials (I/O)
 */
void initPeripherials(void);

/**
 * Read the light sensor
 * @return The ADC value
 */
uint16_t getLightSensor(void);

#ifdef FLAP_POT
/**
 * Read the flap position
 * @return The ADC value
 */
uint16_t getFlapPosition(void);

#endif

/**
 * Beep
 */
void beep(void);

/**
 * Opens/close the green latch
 */
bool lockGreenLatch(bool lock);

/**
 * Opens/close the red latch
 */
bool lockRedLatch(bool lock);

#endif	/* XC_HEADER_TEMPLATE_H */

