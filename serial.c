/*
 * File:   serial.c
 * Author: mdonze
 *
 * Created on 13 November 2018, 14:55
 */

#include "serial.h"
#include <xc.h>
#include <stdio.h>
#include "interrupts.h"

#define SERIAL_TIMEOUT 5

volatile struct RingBuffer rxBuffer;


void initSerial(void)
{
   //Pin for UART
   TRISC7 = 1;
   TRISC6 = 1;
   //Clock divider
   SPBRG = DIVIDER;
   //Receive control register
   RCSTA = 0x0;
   //Serial port enabled
   RCSTAbits.SPEN = 1;
   //Continous receive enable
   RCSTAbits.CREN = 1;
   //TX enabled, high speed mode
   TXSTA = 0x24;
   //Enable interrupts on RX
   RCIE = 1;
   
   rxBuffer.rIndex = 0;
   rxBuffer.uIndex = 0;
   
}

/**
 * Putch for printf support
 * @param byte
 */
void putch(char byte)
{
	while(!TXIF)
		continue;
	TXREG = byte;
}

void putShort(uint16_t v){
    putch(v & 0xFF);
    putch((v>>8) & 0xFF);
}

uint8_t getShort(uint16_t* v)
{
    ms_t start = millis();
    while(rxBuffer.rIndex == rxBuffer.uIndex){
        if((millis()-start)>SERIAL_TIMEOUT){
            //Timeout
            return 1;
        }
    }
    *v = rxBuffer.buffer[rxBuffer.uIndex];
    if(++rxBuffer.uIndex == SER_BUFFER){
        rxBuffer.uIndex = 0;
    }
    while(rxBuffer.rIndex == rxBuffer.uIndex){
        if((millis()-start)>SERIAL_TIMEOUT){
            //Timeout
            return 1;
        }
    }
    *v |= (rxBuffer.buffer[rxBuffer.uIndex]<<8);
    if(++rxBuffer.uIndex == SER_BUFFER){
        rxBuffer.uIndex = 0;
    }
    return 0;
}

uint8_t getByte(uint8_t* v)
{
    ms_t start = millis();
    while(rxBuffer.rIndex == rxBuffer.uIndex){
        if((millis()-start)>SERIAL_TIMEOUT){
            //Timeout
            return 1;
        }
    }
    *v = rxBuffer.buffer[rxBuffer.uIndex];
    if(++rxBuffer.uIndex == SER_BUFFER){
       rxBuffer.uIndex = 0;
    }
    return 0;
}

bool byteAvail(void)
{
    return rxBuffer.rIndex != rxBuffer.uIndex;
}