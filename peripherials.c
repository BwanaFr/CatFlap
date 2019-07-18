/*
 * File:   peripherials.c
 * Author: mdonze
 *
 * Created on 13 November 2018, 15:52
 */
#include <xc.h>
#include "peripherials.h"
#include "interrupts.h"

/**
 * Initialize peripherials (I/O)
 */
void initPeripherials(void)
{
    //All ports to 0
    PORTA = 0x0;
    PORTB = 0x0;
    PORTC = 0x0;
    //No interrupts
    INTCON = 0x0;
    PIE1 = 0x0;
    PIE2 = 0x0;
#ifdef FLAP_POT        
    //RA0, RA1, RA2 and RA3 are analog inputs
    ANSEL = 0xF;
#else
    //RA0, RA2 and RA3 are analog inputs
    ANSEL = 0xD;
#endif
    ANSELH = 0x0;
    //Only RA5 is output
    TRISA = 0xDF;
    //Configure weak pull-ups for PORT B
    WPUB = 0xC1;
    OPTION_REGbits.nRBPU = 0;    
    //RB7, RB6, RB0 are inputs
    TRISB = 0xC1;
    //RC7, RC6 are not used by I/O
    //TODO: RC5 is used for debugging
    TRISC = 0xC0;
    //ADC config (right justified result)
    ADCON1 = 0b10000000;    
    //No CCP2
    CCP2CON = 0x0;
    
    //Configure timer 1 (millis counter)
    T1CONbits.T1CKPS1 = 1;   // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 0;   // bit 4
    T1CONbits.T1OSCEN = 0;   // bit 3 Timer1 Oscillator Enable Control bit 1 = on
    T1CONbits.T1SYNC = 0;    // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
    T1CONbits.TMR1CS = 0;    // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
    T1CONbits.TMR1ON = 1;    // bit 0 enables timer
    TMR1H = TMR1_H_PRES;     // preset for timer1 MSB register
    TMR1L = TMR1_L_PRES;     // preset for timer1 LSB register
    
    //Enable interrupt on timer 1
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}


uint16_t getLightSensor(void)
{
    //Start ADC on channel 0
    ADCON0 = 0b10000001;
    //Wait for capacitor to charge
    __delay_us(1000);
    ADCON0bits.GO_DONE = true;
    while(ADCON0bits.GO_DONE){}
    uint16_t ret = ADRESL;
    ret += (ADRESH<<8);
    //Keep ADC on
    ADCON0 = 0x1;    
    return ret;
}

#ifdef FLAP_POT
uint16_t getFlapPosition(void)
{
    //Start ADC on channel 1
    ADCON0 = 0b10000101;
    //Wait for capacitor to charge
    __delay_us(1000);
    ADCON0bits.GO_DONE = true;
    while(ADCON0bits.GO_DONE){}
    uint16_t ret = ADRESL;
    ret += (ADRESH<<8);
    ADCON0 = 0x1;
    return ret;
}

#endif

void beep(void)
{
    for(uint16_t i=0;i<200;++i){
        BUZZER = 1;
        __delay_us(250);	
		BUZZER = 0;
		__delay_us(250);
    }
}

/**
 * Opens the green latch
 */
bool lockGreenLatch(bool lock)
{
    CL_GL_ENABLE = 1;       //Enable channel 1/2
    RFID_RL_ENABLE = 0;     //Disable the 3/4 output
    if(lock){
        GREEN_LOCK = 0;     //Power the green lock
        COMMON_LOCK = 1;    //Power the green lock
    }else{
        GREEN_LOCK = 1;     //Power the green lock
        COMMON_LOCK = 0;    //Power the green lock
    }
    L293_LOGIC = 1;         //Power the logic
    __delay_ms(500);
    L293_LOGIC = 0;         //Power the logic
    CL_GL_ENABLE = 0;       //Enable channel 1/2
    GREEN_LOCK = 1;         //Put locks to 1 to avoid burning L293_LOGIC I/O
    RED_LOCK = 1;           //Put locks to 1 to avoid burning L293_LOGIC I/O
    COMMON_LOCK = 1;        //Put locks to 1 to avoid burning L293_LOGIC I/O
    return lock;
}

/**
 * Opens the green latch
 */
bool lockRedLatch(bool lock)
{
    RFID_RL_ENABLE = 1;     //Enable channel 1/2
    RFID_EXCT = 1;          //Force RFID to 1 (less consumption)
    CL_GL_ENABLE = 1;       //Enable the 3/4 output
    GREEN_LOCK = 1;         //Force green latch to 1 (to avoid burning I/O)
    if(lock){
        RED_LOCK = 0;       //Power the red lock
        COMMON_LOCK = 1;    //Power the red lock        
    }else{
        RED_LOCK = 1;       //Power the red lock
        COMMON_LOCK = 0;    //Power the red lock
    }
    L293_LOGIC = 1;         //Power the logic
    __delay_ms(500);
    L293_LOGIC = 0;         //Power the logic
    CL_GL_ENABLE = 0;       //Disable channel 1/2
    RFID_RL_ENABLE = 0;     //Disable channel 3/4
    GREEN_LOCK = 1;         //Put locks to 1 to avoid burning L293_LOGIC I/O
    RED_LOCK = 1;           //Put locks to 1 to avoid burning L293_LOGIC I/O
    COMMON_LOCK = 1;        //Put locks to 1 to avoid burning L293_LOGIC I/O
    return lock;
}

