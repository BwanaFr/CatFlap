/*
 * File:   cat.c
 * Author: mdonze
 *
 * Created on 22 November 2018, 13:56
 */


#include <xc.h>
#include "cat.h"
#include "peripherials.h"


uint16_t getConfiguration(uint8_t cfg)
{
    uint16_t ret = 0;
    uint8_t offset = cfg*2;
    //Ensure we read the configuration area
    if(offset<CAT_OFFSET){        
        ret = eeprom_read(offset);
        ret |= (eeprom_read(offset+1)<<8);
    }
    return ret;
}

void setConfiguration(uint8_t cfg, uint16_t value)
{
    uint8_t offset = cfg*2;
    //Ensure we write in the configuration area
    if(offset<CAT_OFFSET){        
        eeprom_write(offset, (value & 0xFF));
        eeprom_write(offset+1, ((value>>8) & 0xFF));
    }    
}

void getCat(Cat* cat, uint8_t slot)
{
    if(slot<CAT_SLOTS){
        uint8_t offset = CAT_OFFSET+slot*sizeof(Cat);
        cat->crc = eeprom_read(offset);
        cat->crc |= (eeprom_read(offset+1)<<8);
        offset +=2;
        for(uint8_t i=0;i<6;++i){
            cat->id[i] = eeprom_read(i+offset);
        }
    }else{
        //Not found
        cat->crc = 0x0;
        for(uint8_t i=0;i<6;++i){
            cat->id[i] = 0x0;
        }
    }
}

/**
 * Save a cat in the eeprom
 * @param cat
 */
uint8_t saveCat(Cat* cat)
{
    uint8_t offset = CAT_OFFSET;
    for(uint8_t i=0;i<CAT_SLOTS;++i){
        uint8_t tCrc = eeprom_read(offset);
        tCrc |= eeprom_read(offset+1);
        if(tCrc == 0x0){
            eeprom_write(offset, cat->crc & 0xFF);
            eeprom_write(offset+1, ((cat->crc>>8) & 0xFF));
            offset +=2;
            for(uint8_t j=0;j<6;++j){
                eeprom_write(j+offset, cat->id[j]);
            }
            return i+1;
        }else if(tCrc == cat->crc){
            //Already stored
            return i+1;
        }
        offset += sizeof(Cat);
    }
    return 0;
}

/**
 * Locate a cat by it's CRC
 * @param crc
 * @return 
 */
bool catExists(Cat* cat, uint16_t* otherCrc)
{
    uint8_t offset = CAT_OFFSET;
    for(uint8_t i=0;i<CAT_SLOTS;++i){
        uint16_t tCrc = eeprom_read(offset);
        tCrc |= (eeprom_read(offset+1) << 8);
        if((tCrc == cat->crc) &&
                (*otherCrc != 0) && (*otherCrc == tCrc)){
            offset +=2;
            for(uint8_t j=0;j<6;++j){
                cat->id[j] = eeprom_read(j+offset);
            }
            return true;
        }
        offset = CAT_OFFSET + i*sizeof(Cat);
    }
    return false;
}

/**
 * Clear all cats in the EEPROM memory
 */
void clearCats(void)
{
    uint8_t offset = CAT_OFFSET;
    for(uint8_t i=0;i<CAT_SLOTS;++i){
        //Only clear CRC
        eeprom_write(offset, 0x0);
        eeprom_write(offset+1, 0x0);
        offset += sizeof(Cat);
    }
    for(uint8_t i=0;i<5;++i){
        beep();
        __delay_ms(100);
    }
}