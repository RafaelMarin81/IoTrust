/*
 * internalI2C.cpp
 *
 * Created: 10/9/2015 11:22:23 PM
 *  Author: smkk
 *
 * This is used only in the variant directory, should not be exported for the user project.
 * It remap the Wire library for the initialization of the ASME and for the user API related to the
 * GPD and SFX component.
 */ 

#include <Arduino.h>
#include "WireIoExt.h"
#include "internalI2C.h"


void internalI2CInit(void){
    WireTemp.begin();
}

uint8_t error=0x45;
uint32_t time, actualTime;
byte readRegister(byte _address, byte regToRead)
{
    WireTemp.beginTransmission(_address);
    WireTemp.write(regToRead);
    WireTemp.endTransmission(false); //endTransmission but keep the connection active

    error = WireTemp.requestFrom(_address, 1); //Ask for 1 byte, once done, bus is released by default

time = millis();
do {
        actualTime = millis();
        if (actualTime-time >= 2000)
            return 0;
    }while(!WireTemp.available()); //Wait for the data to come back
    return WireTemp.read(); //Return this one byte
}

// Writes a single byte (dataToWrite) into regToWrite
bool writeRegister(byte _address, byte regToWrite, byte dataToWrite)
{
    WireTemp.beginTransmission(_address);

    if (!WireTemp.write(regToWrite)) {
        return false;
    }
    if (!WireTemp.write(dataToWrite)) {
        return false;
    }

    uint8_t errorNo = WireTemp.endTransmission(); //Stop transmitting
    return (errorNo == 0);
}

