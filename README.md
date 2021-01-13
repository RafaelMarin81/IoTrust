# IoTrust

## Getting Started

### How this portable Arduino IDE distribution was created

- The directory `tools/arduino-1.8.13/` contains a distributable and portable
  linux `amd64` ArduinoIDE development environment.

- The major advantage of a portable ArduinoIDE distribution like this is that
  you have everything that you need to do development if you clone this
  repository. All the needed libraries and sketches will be there and it will
  make your life easier because you wont break compatibility if you switch
  development environments or boards.

- **NOTE** the procedure described in this section has already been performed
  once at the begining of the project. The guide is kept here for reference.

- The contents of `tools/arduino-1.8.13/` are created as follows:

- Download the distribution from the official webpage:

```
wget https://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz
tar xvf arduino-1.8.13-linux64.tar.xz
```


- Inside `arduino-1.8.13/`, create a directory named `portable`:

```
cd arduino-1.8.13/
mkdir portable
```

- By merely existing, this `portable` directory modifies the base behaviour of
  the ArduinoIDE environment. From now on, all the libraries, compilers,
  utilities, and configuration files are always contained within the portable
  folder.

```
cd arduino-1.8.13/
./arduino
```

- Before compiling, the SmartEverything (SME) Lion Arduino board an libraries
  must be installed.

#### Installation of the board and libraries

**Board Installation**

- The procedure is described in Section 5.2 of the SME Lion datasheet at
  `docs/reference/asme-lion_guide.pdf`. Follow the instructions carefully.

- The Arduino SAMD ARM Cortex M0+ board has been installed in v1.8.11.
- The Partner Arrow Boards have been installed in v2.1.0.

**Library Installation**

- The procedure is described in Section 5.5 of the SME Lion datasheet at
  `docs/reference/asme-lion_guide.pdf`. Follow the instructions carefully.

- The SmartEverything Lion RN2483 library has been installed in v1.3.0.
- The Sodaq_RN2483 library has been installed in v1.1.0.
- The GPS SE868-AS library has been installed in v1.1.1.
- The extEEPROM library has been installed in v3.4.1.

- The BLE RN4870 library must be installed manually from the ZIP file. Download the release with:

```
wget https://github.com/axelelettronica/sme-rn4870-library/archive/v1.0.0.zip 
```

- Go back to the ArduinoIDE and install it via the menu `Sketch` -> `Libraries` ->  `Install Libraries`.


## Contributor

[asvin](https://asvin.io)\
[Odins](https://www.odins.es)\

## License

## Contact

[Mirko Ross](m.ross@asvin.io) from [asvin](https://asvin.io)\
[Rafael Marin-Perez](https://www.odins.es) from [Odins](https://www.odins.es)
