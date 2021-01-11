/*

  Copyright (c) 2014 Arduino.  All right reserved.

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
#include <SPI.h>

#include <ADXRS290.h>
#include <ADXL362.h>

//#include <ecc508.h>

// Change to AtSha204() to use the 204
//AtEccX08 sha = AtEccX08();

void setup(void)
{
	SerialUSB.begin(115200);

    SPI1.begin();
    delay(1000);

	//while (!SerialUSB) {
    //	;
	//}
    //sha.enableDebug(&Serial);

   // adiAccelerometer.begin(ADXL362_CS, &SPI1, ADXL362_EINT);
   // adiAccelerometer.setPowerMode(1);
    adiGyroscope.begin(ADXRS290_CS, &SPI1, ADXRS290_EINT);
    adiGyroscope.standbyModeEnable(false);
    adiGyroscope.tempSensorEnable(true);
}



volatile uint16_t i = 0;
float x, y, z;
float temp;

void loop(void)
{    
    digitalWrite(PIN_LED, HIGH);

	if (SerialUSB.available() > 0) {
		char inChar = SerialUSB.read();
        SerialUSB.print(inChar);
    }
    if (i%10000) {
        SerialUSB.print("Temperature = ");
        SerialUSB.println(temp);

     //   adiAccelerometer.getGxyz(&x, &y, &z);
     //   temp = adiAccelerometer.readTemperature();
        
        SerialUSB.print("Temperature = ");
        SerialUSB.print(temp);
        SerialUSB.print("\tX axis = ");
        SerialUSB.print(x);
        SerialUSB.print("\tY axis = ");
        SerialUSB.print(y);        
        SerialUSB.print("\tz axis = ");
        SerialUSB.println(z);     
        temp = adiGyroscope.readTemperature();
    }   
    ++i;
    delay(100);
        /* If you haven't personalized your device yet, you will recieve
     * this on your serial terminal:
       ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000
       Success

       Otherwise, you'll get actual random bytes.

    if (0 == sha.getRandom(0)){
        Serial.println("Success");
        sha.rsp.dumpHex(&Serial);
    }
    else{
        Serial.println("Failure");
    }
*/
}



/*

#include <Arduino.h>

#include <SPI.h>
#include <ADXL362.h>

ADXL362 xl;

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;

void setup(){
    
	Serial.begin(115200);
    xl.begin(ADXL362_CS);                   // Setup SPI protocol, issue device soft reset
    xl.beginMeasure();              // Switch ADXL362 to measure mode
    
    Serial.println("Start Demo: Simple Read");
}

void loop(){
    
    // read all three axis in burst to ensure all measurements correspond to same sample time
    xl.readXYZTData(XValue, YValue, ZValue, Temperature);
    Serial.print("XVALUE=");
    Serial.print(XValue);
    Serial.print("\tYVALUE=");
    Serial.print(YValue);
    Serial.print("\tZVALUE=");
    Serial.print(ZValue);
    Serial.print("\tTEMPERATURE=");
    Serial.println(Temperature);
    delay(100);                // Arbitrary delay to make serial monitor easier to observe
}

*/