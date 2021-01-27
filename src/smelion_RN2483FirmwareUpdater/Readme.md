# RN2483 Firmware Updater

## Overview


This sketch updates the Microchip RN2483 embedded in the SmartEverything
(SME) Lion to its v1.0.6 RC3. It only requires an ArduinoIDE version
1.8.0+ and all the SME Lion core libraries installed prior to its
execution.

The SME Lion is an Arduino-type board that includes a
Microchip RN2483 LoRaWAN certified module embedded within its integrated
circuit.

The Microchip RN2483 has the feature of allowing the user updating its
firmware, and the latest versions of the firmware - v1.0.5+ - include
LoRaWAN Class C functionality.

Having LoRaWAN Clas A and C is a breakthrough that justifies the
firmware update on its own, regardless of what is your scenario.

Normally, to update the Microchip RN2483's firmware, you would connect
it directly to your computer using a serial port with a serial
transceiver. Also, you would use the software tools provided my the
manufacturer, Microchip, itself. However, the RN2483 included within SME
Lion is soldered to the board, thus connecting it directly to a computer
serial port is not a practical option.

The problem that this sketch solves is that you cannot connect the
Microchip RN2483 embedded within the SME Lion board to your PC directly.
Hence, the sketch is intended to do the same work as the tools provided
by the manufacturer to update the firmware. Instead of running the
program logic within the computer, it runs in the SME Lion
microcontroller itself.


The present code is intended to run exclusively in the SME Lion,
which includes an Atmel SAMD21 MCU, similar to the Arduino Zero and Sodaq
Arduino boards. The architecture is ARM Cortex M0+. Thus, if you try to
port this sketch to other platforms, make sure that it has enough
storage space and check other compatibility indications.

Architecture:

```
+-------------------------------------------------+
|                                                 |
|     Computer                                    |                +--------------------+
|                                                 |                |                    |
|    +-----------------------------------------+  |                | SME Lion           |
|    |ArduinoIDE                               |  |                |                    |
|    |                                         |  |                |  +--------------+  |
|    |  +------------------------------------+ |  |     USB Cable  |  |              |  |
|    |  | smelion_RN2483FirmwareUpdater.ino  | |  +---------------->  |  Microchip   |  |
|    |  |                                    | |  |                |  |  RN2483      |  |
|    |  |                                    | |  |                |  |              |  |
|    |  +------------------------------------+ |  |                |  +--------------+  |
|    |                                         |  |                |                    |
|    +-----------------------------------------+  |                +--------------------+
|                                                 |
+-------------------------------------------------+
```


## Organization


**Sodaq Firmware code**

```
smelion_RN2483FirmwareUpdater.ino
HexFileImage.h
IntelHexParser.cpp
IntelHexParser.h
RN2483Bootloader.cpp
RN2483Bootloader.h
Utils.h
```

These files are taken from the Sodaq project to generate a firmware.

- https://github.com/SodaqMoja/RN2483FirmwareUpdater

This sketch incorporates minimum changes to make it work with the
SME Lion board. Refer to the last commits in the Sodaq Git Repo for
integrating any significant changes into this sketch.

**Main sketch file**


```
smelion_RN2483FirmwareUpdater.ino
```

This is the SME Lion firmware that does all the work. Please note that
the Microchip RN2483 has two different operation modes:

- `Application mode`
- `Bootloaer mode`

Depending on the mode, the communication mechanism changes drastically.
E.g. the baudrate is different, bootloader employs binary commands,
while application mode only uses plain text commands.

The sketch main loop() function tries to do everything automatically.
It detects if the RN2483 is already in bootloader mode, however you can
type `b` during the first 5 seconds in order to go directly into the
bootloader mode logic if that is your case.

**Hex file images**

```
HexFileImage2483_101.h
HexFileImage2483_103.h
HexFileImage2483_106_RC3.h
....
```

These files contain the Microchip RN2483 firmware itself in HEX file
format. The firmwares can be obtained from the Manufacturer webpage and
recently (ca. 2020) from a git repository that contains the source code
for the latest versions (v1.0.6 RC3+).

- https://www.microchip.com/wwwproducts/en/RN2483
- https://github.com/MicrochipTech/RN2xx3_LORAWAN_FIRMWARE

- These files are what would be employed in case of using a direct
  connection to the Microchip RN2483 through serial console.

**Sodaq watchdog files**

```
Sodaq_wdt.h
Sodaq_wdt.cpp
```

These files implement the watchdog features for the Sodaq Compatible
boards, it is mostly useless for the SME Lion, thus it has been
deactivated in the code. They're kept for archival purposes, but they're
not necesary for the SME Lion board.


## Building & Runing

- Install ArduinoIDE v1.8.0+ from the official webpage.
- Install all the SME Lion arduino core libraries, please refer to the
  SME Lion datasheet reference. See Chapter 5 and 6.
  - https://static6.arrow.com/aropdfconversion/5ff647cd30f423703234cbf85de7f2e794f2b199/smarteverythingasmelionuserguide.pdf
  - Alternatively, search for `ASME Lion User Guide`
- Connect the SME Lion board to the computer through USB cable.
- Check that you have selected the right SME Lion board core
  - Menu `Tools` -> `Boards` -> `SmartEverything Lion (Native USB Port)`.
- Check that the SME Lion is connected and the serial port is chosen.
  - Menu `Tools` -> `Port` ->
  - E.g. `/dev/ttyACM0` in Linux, or `COM0` in Windows.
- Load `smelion_RN2483FirmwareUpdater.ino`
- Open the Serial Monitor. Menu `Tools` -> `Serial Monitor`.
- Upload the sketch. `Ctrl+U`.
- Wait until the procedure is finished. It will show its progress on the
  Serial Monitor.


## Dependencies and requirements

- A computer with ArduinoIDE v1.8.0+ installed.

- SmartEverything Lion board.

## Hex Image Selection

In HexFileImage.h you can set which hex file image should be included in
the firmware, to be used for updating the module:
````C
//#define HEXFILE_RN2483_101
//#define HEXFILE_RN2483_103
//#define HEXFILE_RN2903AU_097rc7
//#define HEXFILE_RN2903_098
````

You have to uncomment one of these lines to select the required firmware.

## Other Firmware

You can include any other firmware hex file by opening the hex file in the
text editor of your choice (which should support columns) and adding a
double quote at the beginning of each line and a double quote and a comma
at the end of the line like this:
```
...
:10030000D7EF01F0FFFFFFFF5A82FACF2AF0FBCFB1
:100310002BF0D9CF2CF0DACF2DF0F3CF2EF0F4CF95
...
```
**becomes**
```
...
":10030000D7EF01F0FFFFFFFF5A82FACF2AF0FBCFB1",
":100310002BF0D9CF2CF0DACF2DF0F3CF2EF0F4CF95",
...
```

Then you can copy the result into a block like this:

```C
const char* const TheHexFileNameHere[] = {
  ...
  ":10030000D7EF01F0FFFFFFFF5A82FACF2AF0FBCFB1",
  ":100310002BF0D9CF2CF0DACF2DF0F3CF2EF0F4CF95",
  ...
}

```

and append the code into HexFileImage.h.

##  Step-by-step

After compiling the source code and uploading it to the board you will be able to start the process using a serial terminal.

Just open the Arduino Serial Monitor (at 115200 baud) and you will see this:

```
** SODAQ Firmware Updater **
Version 1.4

Press:
- 'b' to enable bootloader mode
- 'd' to enable debug
.......
```

You have 5 seconds to press any of the shown keys to enable the shown functionality (optional).

Then, the hex file image will be verified while showing the progress:

```
** SODAQ Firmware Updater **
Version 1.4

Press:
 - 'b' to enable bootloader mode
 - 'd' to enable debug
....................

* Starting HEX File Image Verification...
 0% |||||||||||| 25% |||||||||||| 50% |||||||||||| 75% |||||||||||| 100% 
HEX File Image Verification Successful!
```

At this point you should normally see the following message, allowing you to start the update process:

```
* The module is in Application mode: 
RN2483 1.0.1 Dec 15 2015 09:38:09

Ready to start firmware update...
Firmware Image: RN2483_101

Please press 'c' to continue...
```

The update will begin and a similar progress bar as above will be shown. Once the update is complete you can power-cycle the module to boot the new firmware!

## In case something goes wrong

In case there is something wrong after the module's application has been erased you can force the updater to communicate directly with the module's bootloader by pressing 'b' during the 5-seconds boot delay.


## Known Issues

- It seems that sometimes the RN2483 might get stuck into bootloader
mode after the sketch finished. Normally it would be reset by the SME
Lion MCU and put back into *application* mode. If that happens, it will not
answer any text commands (e.g. `sys get ver`, `mac get dr`).

- If something goes wrong, try running the sketch again in bootloader mode,
type a `b` during the first 5 seconds in the Serial Monitor.


## Third Party Software

**Original project sources**

- https://github.com/SodaqMoja/RN2483FirmwareUpdater


**Hex file images**


```
HexFileImage2483_101.h
HexFileImage2483_103.h
HexFileImage2483_106_RC3.h
....
```

These files contain the Microchip RN2483 firmware itself in HEX file
format. The firmwares can be obtained from the Manufacturer webpage and
recently (ca. 2020) from a git repository that contains the source code
for the latest versions (v1.0.6 RC3+).

- https://www.microchip.com/wwwproducts/en/RN2483
- https://github.com/MicrochipTech/RN2xx3_LORAWAN_FIRMWARE

- These files are what would be employed in case of using a direct
  connection to the Microchip RN2483 through serial console.


## References


- https://github.com/SodaqMoja/RN2483FirmwareUpdater
- https://lorawan-hackathon.readthedocs.io/en/latest/index.html

- https://www.microchip.com/wwwproducts/en/RN2483
- https://github.com/MicrochipTech/RN2xx3_LORAWAN_FIRMWARE

## Support

- (2021-01-27) Jesus Sanchez jsanchez@odins.es.



## License

Copyright (c) 2017, SODAQ
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

