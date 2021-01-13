/*
Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
Copyright (c) 2015 Amel-Tech (a division of Amel Srl). All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include "internalI2C.h"

static const uint32_t TWI_CLOCK_SME = 100000;

static void setInitGPS(void)
{
    // Initialize the Serial of the GPS.
    // This is hide to the user that could run again the begin.
    // But it is required to send the message for the STDBY.
    GPS.begin(9600);
    
    // set GPS in standby
    gpsSleep();
}


void resetBaseComponent() {
    digitalWrite(PIN_RESET_COMPONENT, LOW);
    delay(10); // wait 10 mSec.
    digitalWrite(PIN_RESET_COMPONENT, HIGH);
}


volatile     uint8_t actual;

static void ioExtenderInit(void) {
    internalI2CInit();

    actual = readRegister( FXL6408_ADDRESS, FXL6408_DEVICE_ID_REG);
    if ((actual & FXL6408_ID_VALUE) != FXL6408_ID_VALUE) {
        smeInitError |= IOEXT_ERR;
    }
    
    writeRegister(FXL6408_ADDRESS, FXL6408_DEVICE_CONF_PORT_REG, CONF_PORT);
    actual = readRegister( FXL6408_ADDRESS, FXL6408_DEVICE_CONF_PORT_REG);
    if ((actual & CONF_PORT) != CONF_PORT) {
        smeInitError |= IOEXT_CONF_ERR;
    }
    
    writeRegister(FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG, FXL6408_NORMAL_RUN_VALUE);
    actual = readRegister( FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG);
    if ((actual & FXL6408_NORMAL_RUN_VALUE) != FXL6408_NORMAL_RUN_VALUE) {
        smeInitError |= IOEXT_CONF_ERR;
    }
    
    // wait a while and reset the chip
    delay(10);
    writeRegister(FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG, FXL6408_INITIAL_RESET_VALUE);
    actual = readRegister( FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG);
    if ((actual & FXL6408_INITIAL_RESET_VALUE) != FXL6408_INITIAL_RESET_VALUE) {
        smeInitError |= IOEXT_CONF_ERR;
    }
    
    // wait a while and exit from Reset
    delay(20);
        writeRegister(FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG, FXL6408_NORMAL_RUN_VALUE);
        actual = readRegister( FXL6408_ADDRESS, FXL6408_DEVICE_OUT_STATE_PORT_REG);
        if ((actual & FXL6408_NORMAL_RUN_VALUE) != FXL6408_NORMAL_RUN_VALUE) {
            smeInitError |= IOEXT_CONF_ERR;
        }
}


void initVariant() {
    
    // initialization the two Yellow Led
    LED_YELLOW_ONE_INIT;
    LED_YELLOW_TWO_INIT;

    // Light Led OFF
    ledYellowOneLight(LOW);
    ledYellowTwoLight(LOW);

    
    // initialize The EXT_PWR Pin as input
    // it will be HIGH when the battery is not connected
    pinMode(PIN_EXT_PWR, INPUT_PULLUP); 
    
    // initialize the battery monitor
    pinMode(PIN_BATT_MON, INPUT);
    pinMode(PIN_LIPO_MON, INPUT);
    pinMode(PIN_ENA_MON, OUTPUT);
    digitalWrite(PIN_ENA_MON, LOW);  // disable the battery monitor
    
    //initialize the reset pin   
    pinMode(PIN_RESET_COMPONENT, OUTPUT);
    digitalWrite(PIN_RESET_COMPONENT, HIGH);
    
    
    // reset the base component
    resetBaseComponent();
    
    // put GPS in stdby to save power
    setInitGPS();

    ioExtenderInit();
}
