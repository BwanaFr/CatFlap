#include <xc.h>         /* XC8 General Include File */

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "interrupts.h"
#include "peripherials.h"
#include "user.h"          /* User funct/params, such as InitApp */
#include "serial.h"
#include <stdio.h>
#include "rfid.h"
#include "cat.h"

/**
 * time to keep door open
 */
#define OPEN_TIME 5000

/**
 * Number of milliseconds
 * between light sensor read
 */
#define LIGHT_READ_PERIOD 5000

#ifdef FLAP_POT
/**
 * Number of milliseconds
 * between flap potentiometer read
 */
#define FLAP_POT_READ_PERIOD 200

#endif

/**
 * Operating mode of flap
 */
#define MODE_NORMAL 0
#define MODE_VET 1
#define MODE_CLOSED 2
#define MODE_NIGHT 3
#define MODE_LEARN 4
#define MODE_CLEAR 5
#define MODE_OPEN 6

/**
 * Defines for button handling
 */
#define GREEN_PRESS 1
#define RED_PRESS 2
#define BOTH_PRESS 3

#define CMD_STATE_IDLE 0
#define CMD_STATE_STAT 1
#define CMD_STATE_MODE 2 
#define CMD_STATE_SETTING 3

//Operation mode
static uint8_t opMode = MODE_NORMAL;    
//Is the out locked?
static bool outLocked = false;
//Is the in locked
static bool inLocked = false;
//Light sensor value
static uint16_t light = 0;
//Light sensor threshold
static uint16_t lightThd = 0;    
#ifdef FLAP_POT
//Flap position
static uint16_t flapPos = 0;
//Flap position tolerance
static uint16_t flapPosTol = 30;
//Flap position IDLE
static uint16_t flapPosIdle = 512;
//Flap open inner 
static bool flapInner = false;
//Flap open outer
static bool flapOuter = false;
#endif

/**
 * Switch flap operating mode
 * @param mode
 */
void switchMode(uint8_t mode){    
    switch(mode){
        case MODE_NIGHT:
        case MODE_NORMAL:
        case MODE_LEARN:
        case MODE_CLEAR:
            //Cat is allowed to go out
            outLocked = lockRedLatch(false);
            inLocked = lockGreenLatch(true);
            break;
        case MODE_VET:
        case MODE_CLOSED:
            //Cat cannot go out
            outLocked = lockRedLatch(true);
            inLocked = lockGreenLatch(true);
            break;
        case MODE_OPEN:
            //Free party mode
            outLocked = lockRedLatch(false);
            inLocked = lockGreenLatch(false);
            break;
        default:
            //Cat is allowed to go out
            outLocked = lockRedLatch(false);
            inLocked = lockGreenLatch(true);
            mode = MODE_NORMAL;
            break;
    }
    opMode = mode;
}

/**
 * Handles button press
 * @param time Time of press
 * @return Button status
 */
uint8_t handleButtons(ms_t *time){
    static bool greenPrev = true;
    static bool redPrev = true;
    static ms_t start = 0;
    static bool bothPressed = false;
    uint8_t ret = 0;
    bool green = GREEN_BTN;
    bool red = RED_BTN;
    ms_t now = millis();
    ms_t elapsed = now-start;
    if(!green && greenPrev){
        //Rising edge on green
        start = millis();
        bothPressed = false;
    }else if(green && !greenPrev){
        //Falling edge on green
        //Is the red button pressed?
        if(bothPressed){
            ret = BOTH_PRESS;
        }else{
            ret = GREEN_PRESS;
        }
    }else if(!green && !greenPrev){
        //Button pressed
        if(!red){
            bothPressed = true;
        }
    }else if(!red && redPrev){
        //Rising edge on red only
        start = millis();
        bothPressed = false;
    }else if(red && !redPrev){
        //Falling edge on red
        //Is the red button pressed?
        if(!green){
            ret = BOTH_PRESS;
        }else{
            ret = RED_PRESS;
        }
    }else if(!red && !redPrev){
        //Button pressed
        if(!green){
            bothPressed = true;
        }
    }
    *time = elapsed;
    greenPrev = green;
    redPrev = red;
    return ret;
}

/**
 * Learn a new cat and save it to eeprom
 */
void learnCat(void)
{
    uint8_t r = 0;
    uint16_t crcRead;    
    ms_t t = millis();
    ms_t now = 0;
    uint16_t i =0;
    Cat cat;
    do{                
        r = readRFID(&cat.id[0], 6, &cat.crc, &crcRead);
        if((r ==0) && (cat.crc == crcRead) && (crcRead != 0)){        
            uint8_t slot = saveCat(&cat);
            if(slot>0){
                //Saved successfully
                beep();
                break;
            }
        }
        ++i;
        if(i>9){
            i = 0;
            GREEN_LED = !GREEN_LED;
        }
        __delay_ms(20);
        now = millis();
    }while((now-t)<30000);
    GREEN_LED = 0;
}

/**
 * Build a bit pattern containing all status
 * Bit 0 : In lock (1 means locked)
 * Bit 1 : Out locked
 * Bit 2 : Flap open to go in (based on pot)
 * Bit 3 : Flap open to go out (based on pot) 
 * @return A bit pattern 
 */
uint16_t buildStatusBits(){
    uint16_t ret = 0;
    if(inLocked){ ret = 0x1; }
    if(outLocked){ ret |= 0x2; }
#ifdef FLAP_POT
    if(flapInner){
        //Flap is open inner direction
        ret |= 0x4;
    }else if(flapOuter){
        //Flap is open closed direction
        ret |= 0x8;
    }
#endif
    return ret;
}

void printStatus(){
    putch('A');
    putch('M');
    putch(opMode);
    putch('L');
    putShort(light);
    putch('P');
#ifdef FLAP_POT
    putShort(flapPos);
#else
    putShort(0);
#endif            
    putch('S');
    putShort(buildStatusBits());
    putch('\n');
}

/**
 * Handle all serial communication with external
 */
void handleSerial(){
    uint8_t b = 0;    
    //Do we received a command?
    if(byteAvail()){
        uint8_t c = 0;
        if(getByte(&c) == 0){
            switch(c){
                case 'S':
                    //Get status
                    printStatus();
                    break;
                case 'C':
                    //Change/read a configuration
                    //read/write?
                    if(getByte(&b) == 0){                
                        uint8_t index = 0;
                        //Get parameter index
                        if(getByte(&index) == 0){
                            if(b == 'S'){
                                //Set the configuration                    
                                uint16_t value = 0;
                                if(getShort(&value) == 0){
                                    setConfiguration(index, value);
                                    putch('A');
                                    putch('C');
                                    putch(index);
                                    putch('V');
                                    putShort(value);
                                    putch('\n');
                                    switch(index){
                                        case LIGHT_CFG:
                                            lightThd = value;
                                            break;
                                        case FLAP_POS_IDLE:
                                            flapPosIdle = value;
                                            break;
                                        case FLAP_POS_MARGIN:
                                            flapPosTol = value;
                                            break;
                                        default:
                                            ;
                                    }
                                }
                            }else{
                                //Read the configuration
                                putch('A');
                                putch('C');
                                putch(index);
                                putch('V');
                                putShort(getConfiguration(index));
                                putch('\n');
                            }
                        }else{
                            printf("AE\n");
                        }
                    }
                    break;
                case 'M':
                    //Change mode
                    if(getByte(&b) == 0){
                        if(b<=MODE_OPEN){
                            switchMode(b);
                            putch('A');
                            putch('M');
                            putch(opMode);
                            putch('\n');            
                        }
                    }
                    break;
                default:
                    //Not handled, ignore it
                    return;
            }
        }
    }
}

/**
 * Send cat ID by serial
 * @param c
 */
void printCat(Cat* c)
{
    putch('E');
    for(uint8_t i=0;i<6;++i){
        putch(c->id[i]);
    }
    putch('\n');
}

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
void main(void)
{
    uint8_t r = 0;
    Cat c;
    uint16_t crcRead;
    ms_t btnPress = 0;    
    /* Initialize I/O and Peripherals for application */
    InitApp();
    lightThd = getConfiguration(LIGHT_CFG);    
    if(lightThd>1023){
        lightThd = 512;
    }
    switchMode(MODE_NORMAL);
    ms_t lastLightRead = millis();
#ifdef FLAP_POT
    ms_t lastFlapRead = lastLightRead;
    flapPosIdle = getConfiguration(FLAP_POS_IDLE);
    flapPosTol = getConfiguration(FLAP_POS_MARGIN);
#endif
    while(1)
    {   
        ms_t ms = millis();
        if((ms-lastLightRead)>LIGHT_READ_PERIOD){
            light = getLightSensor();           
            lastLightRead = ms;
        }
#ifdef FLAP_POT
        if((ms-lastFlapRead)>FLAP_POT_READ_PERIOD){
            flapPos = getFlapPosition();
            bool doUpdate = false;
            if(flapPos > (flapPosIdle+flapPosTol)){
                //Flap is open inner direction (open)
                if(!flapInner){
                    flapInner = true;
                    flapOuter = false;
                    doUpdate = true;
                }
            }else if(flapPos < (flapPosIdle-flapPosTol)){
                //Flap is open outer direction
                if(!flapOuter){
                    flapOuter = true;
                    flapInner = false;
                    doUpdate = true;
                }
            }else{
                doUpdate = flapInner | flapOuter;
                flapOuter = false;
                flapInner = false;
            }
            if(doUpdate){
                printStatus();
            }
            lastFlapRead = ms;
        }
#endif
        bool doOpen = false;
        switch(opMode){
            case MODE_NORMAL:             
                doOpen = true;
                RED_LED = 0;
                GREEN_LED = 0;
                break;
            case MODE_VET:
                GREEN_LED = 0;
                //Blink red led
                RED_LED = ((ms>>9) & 0x1);
                doOpen = true;
                break;
            case MODE_CLOSED:
                doOpen = false;
                //Blink both leds
                RED_LED = ((ms>>9) & 0x1);;
                GREEN_LED = ((ms>>9) & 0x1);
                break;
            case MODE_LEARN:
                learnCat();
                switchMode(MODE_NORMAL);
                break;
            case MODE_CLEAR:
                clearCats();
                switchMode(MODE_NORMAL);
                break;
            case MODE_OPEN:
                RED_LED = 1;
                GREEN_LED = 1;
                doOpen = false;
                break;
            case MODE_NIGHT:
                //Tests if light is not enough
                //More is darker
                if((light>lightThd) && !outLocked){
                    outLocked = lockRedLatch(true);
                    lockGreenLatch(true);
                }else if((light<(lightThd-5)) && outLocked){
                    outLocked = lockRedLatch(false);
                    lockGreenLatch(true);
                }
                GREEN_LED = outLocked;
                RED_LED = 1;
                doOpen = true;
                break;
            default:
                switchMode(MODE_NORMAL);
                doOpen = false;
                break;
        }
        //If open is allowed
        if(doOpen){
            //Read RFID chip
            r = readRFID(&c.id[0], 6, &c.crc, &crcRead);
            if(r == 0 && catExists(&c, &crcRead)){
                //Read ok and found in EEPROM
                beep();
                inLocked = lockGreenLatch(false);
                printCat(&c);
                __delay_ms(OPEN_TIME);
                inLocked = lockGreenLatch(true);                
            }
            c.crc = 0x0;
            //Relax
            __delay_ms(20);
        }
        
        //Handle buttons modes
        switch(handleButtons(&btnPress)){
            case GREEN_PRESS :
                if(btnPress>10000){
                    switchMode(MODE_LEARN);
                }
                break;
            case RED_PRESS :
                if(btnPress>5000){
                    if(opMode == MODE_VET){
                        switchMode(MODE_NORMAL);
                    }else{
                        switchMode(MODE_VET);
                    }
                }else if(btnPress<2000){
                    if(opMode == MODE_NIGHT){
                        switchMode(MODE_NORMAL);
                    }else{
                        switchMode(MODE_NIGHT);                        
                    }
                }
                break;
            case BOTH_PRESS :
                /*if((btnPress>2000) && (btnPress<30000)){
                    //TODO: Extended mode, to be implemented
                }else*/ if(btnPress>30000){
                    switchMode(MODE_CLEAR);                    
                }
                break;              
        }
        
        //Handle serial comm
        handleSerial();
    }
}

