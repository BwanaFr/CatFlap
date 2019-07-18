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
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CAT_INCLUDED_H
#define	CAT_INCLUDED_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <stdbool.h>

//Keep first 128 bytes for global settings
#define CAT_OFFSET 128
//(256-128)/8
#define CAT_SLOTS 16

//Offset of configuration
//Light threshold value
#define LIGHT_CFG 0
#ifdef FLAP_POT
    //Flap middle position
    #define FLAP_POS_IDLE 1
    //Flap middle position margin
    #define FLAP_POS_MARGIN 2
#endif
/**
 Define a cat in the 
 **/
typedef struct{
    uint16_t crc;   //Chip CRC
    uint8_t id[6];  //Chip ID    
}Cat;

/**
 * Gets a configuration
 * @param cfg
 * @return 
 */
uint16_t getConfiguration(uint8_t cfg);

/**
 * Sets a configuration
 * @param cfg
 * @param value
 */
void setConfiguration(uint8_t cfg, uint16_t value);

/**
    Get a cat slot
 */
void getCat(Cat* cat, uint8_t slot);

/**
 * Save a cat in the eeprom
 * @param cat
 */
uint8_t saveCat(Cat* cat);

/**
 * Locate a cat by it's CRC
 * @param cat cat structure
 * @param otherCrc Second CRC to be checked
 * @return 
 */
bool catExists(Cat* cat, uint16_t* otherCrc);

/**
 * Clear all cats in the EEPROM memory
 */
void clearCats(void);

#endif	/* XC_HEADER_TEMPLATE_H */


