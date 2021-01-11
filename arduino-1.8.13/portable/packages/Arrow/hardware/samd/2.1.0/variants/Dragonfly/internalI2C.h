/*
 * internalI2C.h
 *
 * Created: 10/9/2015 11:22:23 PM
 *  Author: smkk
 */ 


#ifndef INTERNALI2C_H_
#define INTERNALI2C_H_

#define BUFFER_LENGTH 32

#define FXL6408_ADDRESS                     0x43
#define FXL6408_DEVICE_ID_REG               0x01
#define FXL6408_DEVICE_CONF_PORT_REG        0x03
#define FXL6408_DEVICE_OUT_STATE_PORT_REG   0x05

#define FXL6408_ID_VALUE        0xA0

#define CONF_PORT     0b11110000
#define INIT_PORT     0b11110000


#define ANTENNA_RST_PIN 0x01
#define BLE_RST_PIN     0x02
#define GPS_RST_PIN     0x04
#define GPS_FORCE_ON    0x08
#define BLE_P10_PIN     0x10
#define BLE_P11_PIN     0x20
#define BLE_P32_PIN     0x40
#define BLE_P33_PIN     0x80

#define FXL6408_NORMAL_RUN_VALUE    ((ANTENNA_RST_PIN|BLE_RST_PIN|GPS_RST_PIN|GPS_FORCE_ON))
#define FXL6408_INITIAL_RESET_VALUE (~((ANTENNA_RST_PIN|BLE_RST_PIN|GPS_RST_PIN)))



// Wire initialization function
void internalI2CInit(void);

byte readRegister(byte _address, byte regToRead);

// Writes a single byte (dataToWrite) into regToWrite
bool writeRegister(byte _address, byte regToWrite, byte dataToWrite);


#endif /* INTERNALI2C_H_ */

