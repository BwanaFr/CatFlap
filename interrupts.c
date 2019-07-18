#include <xc.h>         /* XC8 General Include File */

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "interrupts.h"
#include "serial.h"
#include "peripherials.h"

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/
static volatile ms_t millisValue=0;

void __interrupt () isr(void)
{
    if(TMR1IF && TMR1IE){
        TMR1H = TMR1_H_PRES;             // preset for timer1 MSB register
        TMR1L = TMR1_L_PRES;             // preset for timer1 LSB register        
        TMR1IF = 0;
        ++millisValue;
    }else if(RCIF){
        rxBuffer.buffer[rxBuffer.rIndex] = RCREG;
        RCIF = 0;
        if(++rxBuffer.rIndex == SER_BUFFER){
            rxBuffer.rIndex = 0;
        }
    }
}

ms_t millis(void)
{
    return millisValue;
}
