/*
* Copyright (c) 2015 SODAQ. All rights reserved.
*
* This file is part of Sodaq_RN2483.
*
* Sodaq_RN2483 is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation, either version 3 of
* the License, or(at your option) any later version.
*
* Sodaq_RN2483 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with Sodaq_RN2483.  If not, see
* <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
// #include <Sodaq_RN2483.h>

// MBili
#define debugSerial Serial
// Autonomo
//#define debugSerial SerialUSB
#define loraSerial iotAntenna



void setup()
{

	debugSerial.begin(115200);
	loraSerial.begin(57600);
	delay(100);

	while (!debugSerial)
		;



	debugSerial.println("READY");
}

void loop()
{
  while (debugSerial.available()) 
  {
    char c = debugSerial.read();
    loraSerial.write(c);
    debugSerial.write(c); /* ECHO ON */
  }

  while (loraSerial.available()) 
  {
    debugSerial.write((char)loraSerial.read());
  }
}
