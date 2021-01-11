# sme-arduino-core

Arduino core for Smart Everything board

---
## v2.1.0 14-Dec-2016 Release
Error Fixed on this release:

- Spi inizializaton error fixed in Lion
- ATN Pin definition added for Lion & Fox3

## v2.0.0 Fourth Release 26-Oct-2016
End of Support for SmartEverything Fox

- first release for  SmartEverything:
	    Lion
		Fox3
		Base core for the SmartEverything NXP-ST Shield
			[https://github.com/axelelettronica/sme-nxp-st-library](https://github.com/axelelettronica/sme-nxp-st-library)


##v1.2.0. Third Release 11-May-2015
- Added SmartTutto unit .
- Internal change to move the board from AMEL-Tech to Arrow in the BoardManager.
- Minor change in variantInit to make the 2 unit compatible.


## v1.1.0 Second Release 18-Dec-2015
* Add new function to flash the RGB LED light for X milliseconds<br>
    `void flashRGBLed(uint32_t color, uint32_t time_in_ms);`<br><br>
* Added new GPS & SFX commands<br>
    `gpsForceOn() = send an HW wakeup to GPS`<br>
    `sfxSleep()   = put SFX in sleep`<br>
    `sfxWakeup()  = send an HW wakeup to SFX`<br>



## v1.0.0 First Release
<br><br><br>

---
## Documentation

* **[Installing Arduino IDE Guide](https://www.arduino.cc/en/Guide/HomePage)** - How to install The Arduino IDE
<br><br><br>

---
## License Information


Copyright (c) IOTEAM S.r.l. All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
