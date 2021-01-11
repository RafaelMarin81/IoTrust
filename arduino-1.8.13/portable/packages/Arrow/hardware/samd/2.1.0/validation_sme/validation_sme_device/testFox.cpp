/*
  SerialCommunication.ino

  Example demonstrating a BLE Bidirectional communication between a PC/Smartphone App (Central BLE device)
  and the SmartEveryting USB console (Peripheral BLE)

  - Serial -> BLE : Any character edited on the USB Arduino console will be sent via BLE  
                    on the attribute 0xFFF4 to the central device.
                    NOTICE: The Host needs to subscribe to 0xFFF4 attribute to get notified.

  - BLE -> Serial : Any character written by the Central device, on the writable attribute 0xFFF3, 
                    is showed on the Arduino console.
                    NOTICE: The protocol to be used is <len> <char 1> <char 2> ... <char len>

  Created: 07/07/2016 10:32:11 PM

  Author: development@axelelettronica.it

 */
#include <Arduino.h>

void setup() {
    

}

void loop() {
       delay(1000);
       ledYellowTwoLight(HIGH); // turn the LED on
      // lora.macTxCmd("0123", 1, TX_ACK);
       // SEVE
       //lora.macTxCmd("0123",1);
       delay(1000);
       ledYellowTwoLight(LOW); // turn the LED off
}
