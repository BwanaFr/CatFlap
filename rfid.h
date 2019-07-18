/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef RFID_INCLUDED_H
#define	RFID_INCLUDED_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define NO_CARRIER 1
#define NO_HEADER 2
#define BAD_START 3
#define BAD_CRC 4

/**
 * Read RFID tag
 * @param id Array to store ID of tag
 * @param len Length of id
 * @param crcComputed The CRC computed from ID
 * @param crcRead The CRC read in packet
 * @return 0 on success
 */
uint8_t readRFID(uint8_t* id, uint8_t len, uint16_t* crcComputed,
        uint16_t* crcRead);

void setRFIDPWM(bool on);

#endif	/* XC_HEADER_TEMPLATE_H */

