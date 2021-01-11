
#define ARDUINO_MAIN
#include "Arduino.h"
#include <Wire.h>
#include "extEEPROM.h"


extEEPROM myEEPROM(kbits_256, 1, 64, 0x57);


void setup(void)
{
    SerialUSB.begin(115200);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    while (!SerialUSB) {
        ;
    }

    byte i2cStat = myEEPROM.begin(twiClock100kHz);
    if ( i2cStat != 0 ) {
        SerialUSB.println("I2C Problem");
    }

    SerialUSB.println("EEPROM Memory commands:  read:(a)(l)(r) , write:(a)(d)(w), next read data (n)");
    SerialUSB.println("- Commands TO PRESS:");
    SerialUSB.println("\t a : memory address to read / write");
    SerialUSB.println("\t p : set Paperino");
    SerialUSB.println("\t t : set Topolino");
    SerialUSB.println("\t r : read command");
    SerialUSB.println("\t w : write command");
}
#define PAPERINO "PAPERINO"
#define TOPOLINO "TOPOLINO"
#define PAPERINO_LEN  8
#define TOPOLINO_LEN  8

unsigned long address = 0x100;
const unsigned int maxDataSize = 1024;  //0x8000;   // 32 k bytes (32768 = 0x8000) = 256 kbits

byte e2PromData[maxDataSize] = {'p','i','p', 'p', 'o'};
unsigned int dataSize = 5;

void eprom_read_write(bool write)
{
    byte i2cStat = 0;
    if (write) {
        i2cStat = myEEPROM.write(address, e2PromData, dataSize);
        } else {
        memset(e2PromData, 0, maxDataSize);
        i2cStat = myEEPROM.read(address, e2PromData, dataSize);
    }
    
    if ( i2cStat != 0 ) {
        //there was a problem
        SerialUSB.print("I2C Problem: ");
        if ( i2cStat == EEPROM_ADDR_ERR) {
            SerialUSB.println("Wrong address");
            } else {
            SerialUSB.print("I2C error: ");
            SerialUSB.print(i2cStat);
            SerialUSB.println("");
        }
    }
}


void parse(char inChar)
{
    const char addr_len = 5;
    char       addr_char[addr_len] = "";
    const char data_len = 3;
    char       data_char[data_len] = "";
    char       size_char[data_len] = "";
    char       inc = 0, i = 0, j = 0;

    switch (inChar) {
        case 'p':
        SerialUSB.println("PAPERINO SET ");
        memcpy(e2PromData, PAPERINO, PAPERINO_LEN);
        dataSize = PAPERINO_LEN;
        break;
        
        case 't':
        SerialUSB.println("TOPOLINO SET ");
        memcpy(e2PromData, TOPOLINO, TOPOLINO_LEN);
        dataSize = TOPOLINO_LEN;
        break;
        
        case 'a':
        SerialUSB.print("Insert Address as 4 Hex chars (without '0x'):  ");
        i=0;
        while (i < 4) {
            while (SerialUSB.available() <= 0)
            ;
            inc = SerialUSB.read();

            if (inc == 'q')
            return;

            addr_char[i] = inc;
            ++i;
        }
        address = (unsigned long)strtol(addr_char, NULL, 16);
        SerialUSB.println(address);
        break;

        case 'd':
        SerialUSB.print("Insert Hex data sequence (without '0x'), return to enter: ");
        memset(e2PromData, 0, maxDataSize);
        while (true) {
            while (SerialUSB.available() <= 0)
            ;
            inc = SerialUSB.read();
            if (inc == 'q')
            return;
            if (inc == '\r' || inc == '\n')
            break;

            if (inc >= 'a' && inc <= 'f')
            e2PromData[j] += inc-'a'+10;
            else  if (inc >= 'A' && inc <= 'F')
            e2PromData[j] += inc-'A'+10;
            else if (inc >= '0' && inc <= '9')
            e2PromData[j] += inc-'0';
            else return;

            if (i%2) {
                j++;
                } else {
                e2PromData[j] = e2PromData[j] << 4;
            }
            i++;
        }
        dataSize = j;
        SerialUSB.println(dataSize);
        SerialUSB.println("");
        break;
        case 'l':
        SerialUSB.print("Insert data len as 2 Hex chars (without '0x'): ");
        while (i < 2) {
            while (SerialUSB.available() <= 0)
            ;
            inc = SerialUSB.read();
            if (inc == 'q')
            return;

            size_char[i] = inc;
            ++i;
            if (inc == '\n') {
                return;
            }
        }

        dataSize = (unsigned int)strtol(size_char, NULL, 16);
        SerialUSB.println(dataSize);
        break;


        case 'n':
        address += dataSize;
        /* FALLTHROUGH */
        case 'r':
        SerialUSB.print("reading address: ");
        SerialUSB.println(address, HEX);

        eprom_read_write(false);
        for (i = 0; i < dataSize ; ++i) {
            SerialUSB.print(e2PromData[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();

        break;

        case 'w':
        SerialUSB.print("writing at address: ");
        SerialUSB.print(address, HEX);
        SerialUSB.print(", len: ");
        SerialUSB.println(address, dataSize);
        for (i = 0; i < dataSize ; ++i) {
            SerialUSB.print(e2PromData[i], HEX);
            SerialUSB.print(" ");
        }
        eprom_read_write(true);
        SerialUSB.println();

        break;
        case 'T':
        SerialUSB.println("Memory test: writing and verifying the whole memory");
        break;

        default:
        break;
    }
}


void loop(void)
{
    if (SerialUSB.available() > 0) {
        char inChar = SerialUSB.read();
        SerialUSB.print(inChar);
        parse(inChar);
    }

    ledYellowOneLight(HIGH);
    ledYellowTwoLight(LOW);
    delay(1000);
    ledYellowOneLight(LOW);
    ledYellowTwoLight(HIGH);
    delay(1000);
}
