/* 
 * File:   rfid.c
 * Author: mdonze
 *
 * Created on 13 November 2018, 15:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "rfid.h"
#include "peripherials.h"
#include "interrupts.h"

static bool nextBit = false;

uint16_t readRFIDADCS(void){    
    ADCON0 = 0b10001001;    
    ADCON0bits.GO_DONE = 1;
    while(ADCON0bits.GO_DONE){}
    uint16_t ret = ADRESL;
    ret += ((ADRESH & 0x3) <<8) ;            
    return ret;
}

uint16_t readRFIDADC(void){
    ADCON0 = 0b00001001;    
    ADCON0bits.GO_DONE = 1;
    while(ADCON0bits.GO_DONE){}
    uint16_t ret = ADRESL;
    ret += ((ADRESH & 0x3) <<8) ;   
    return ret;
}

bool readRFIDBitADC(void){
    
    return readRFIDADC()>512;
}

bool readBit(){
    bool ret = true;
    bool next = nextBit;
    for(uint8_t i=0;i<16;++i){
        while(!PIR1bits.TMR2IF);
        PIR1bits.TMR2IF = 0;
        if(((i>7) && (i<11))){
           bool b = readRFIDBitADC();
           if(b != next){
                ret = false;
            } 
            nextBit = !b;
        }
    }   
    return ret;
}

void setRFIDPWM(bool on)
{
    if(on){

        //Power the analog op-amp
        LM324_PWR = 1;
        //Enable ADC                
        ADCON0bits.ADON = 1;
        //Disable output
        TRISCbits.TRISC2 = 1;
        //we will use a prescaler of 1:1
        //TODO: Compute this according to _XTAL_FREQ
        PR2 = 0x24;     
        //PWM mode
        CCP1CON = 0b00011100;
        //50% duty cycle        
        CCPR1L = 0b00010010;
        PIR1bits.TMR2IF = 0;
        T2CON = 0b00001100; //Timer 2 ON, no prescaler. Post scaler to 1:2
        //Wait for the new PWM cycle
        //while(!PIR1bits.TMR2IF);
        //PIR1bits.TMR2IF = 0;
        //Enable output
        TRISCbits.TRISC2 = 0;
        //Power the L293
        CL_GL_ENABLE = 0;   //No output on channel 1/2
        RED_LOCK = 1;       //Low output on red lock
        RFID_RL_ENABLE = 1; //Enable the 3/4 output
        L293_LOGIC = 1;     //Power the logic
        //Wait for analog + ADC to be stable
        __delay_ms(2);
    }else{
        //Disable output
        L293_LOGIC = 0;
        LM324_PWR = 0;
        RFID_RL_ENABLE = 0;
        TRISCbits.TRISC2 = 1;
        T2CONbits.TMR2ON = 0;
        PIR1bits.TMR2IF = 0;
        CCP1CON = 0x0;
        //Disable ADC
        //ADCON0bits.ADON = 0;
    }
}

bool waitEdge(){
    
    bool v = readRFIDBitADC();
    while(v == readRFIDBitADC()){}
    return !v;
}

/**
 * Synchronize the RFID
 * @return 
 */
uint8_t syncRFID(void){
    //Wait for header    
    ms_t t = millis();
    while((millis()-t)<100){
        if(readRFIDADCS()>200){
            continue;                    
        }
        nextBit = waitEdge();        
        bool found = true;
        for(uint8_t k=0;k<10;++k){
            if(readBit()){
                found = false;
                break;
            }
        }
        if(found){
            return 0;
        }        
    }
    return NO_HEADER;
}

uint8_t readRFIDByte(uint8_t* d)
{
    *d = 0x0;
    //Start bit
//    if(readBit()){
    readBit();
        for(uint8_t i=0;i<8;++i){            
            if(readBit()){
                *d |= (1<<i);
            }            
        }
        return 0;
/*    }else{
        return BAD_START;
    }*/
}


uint16_t get_crc_ccit(uint16_t crc, uint8_t d){
  int8_t i;
  uint16_t ret = crc;
  for(i=0;i<8;++i){
      bool b = ((d>>i) & 1);
      bool c15 = ((ret >> 15 & 1) == 1);
      ret <<=1;
      if(c15 ^ b){
          ret ^= 0x1021;
      }
  }
  return ret;
}


uint16_t crc(uint8_t *p, uint8_t len){
    uint16_t crcTmp = 0;
    uint16_t ret = 0;
    /* while there is more data to process */
    while (len-- > 0) {
        crcTmp = get_crc_ccit(crcTmp, *p);
        p++;
    }
    ret = 0;
    //Reverse bits
    for(uint8_t i=0;i<16;++i){
        ret |= ((crcTmp>>i) & 0x1) << (15-i);
    }    
    return ret;
}

uint8_t readRFID(uint8_t* id, uint8_t len, uint16_t* crcComputed,
        uint16_t* crcRead)
{
    uint8_t r=0;
    //Put excitation on
    setRFIDPWM(true);
    //Wait for RFID synchro (up to 100ms)
    r = syncRFID();    
    if(r == 0){          
        //Read only 10 bytes (after header)
        uint8_t bytes[10];
        for(uint8_t k=0;k<10;++k){
            r = readRFIDByte(&bytes[k]);
            if(r != 0){ 
                break;
            }
        }
        if(r == 0){      
            //Copy ID to array
            for(uint8_t i=0;i<len;++i){
                id[i] = bytes[i];                    
            }
            //Compute CRC
            *crcRead = bytes[8];
            *crcRead |= (bytes[9]<<8);
            *crcComputed = crc(bytes, 8); 
            if(*crcRead != *crcComputed){
                r = BAD_CRC;
            }
        }
    }
    //Put excitation off
    setRFIDPWM(false);
    return r;
}