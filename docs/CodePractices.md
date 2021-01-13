# Code Practices

The project involves multiple parties, so it is essential to have common guildlines to keep the code easy to understand and style consistent. This document serves as the reference for code practices and styles for the project.

## Table of Contents

* [Development](#development)
* [Naming](#naming)
* [Formating](#formating)

## Development

Use git branch feature. Create a branch for specific user and feature.

## Naming

This section outlines the naming guildlines for variables, functions and classes.

* Use **PascalCasing** for class name

```C++
class TestClass{

}
```

* Use **camelCasing** for method names, method arguments and local variables.

```C++
class TestClass{
    public:
        void testMethod(int testVar, int temp){
            int roomTemp = temp;
        }
}
```

* Avoid using abbreviations for identifiers.

```C++
int pinMd = HIGH;  // avoid
int pinMode = HIGH; // correct
```

* Use **UPPERCASE** for `#define` macros and global constants

```C++
#define DEBUG 1
int VERSION = 1.8;
```

## Comments

## Formating

* Explain the code at start of a file

```C++
/***********************************************************************
 * File Name    : TestFile.h                                          *
 * Author       : TestAuthor(testauthor@test.com)                     *
 *                                                                    *
 * This is a test file. It defines many tests. A test  is defined in  *
 * a specific function                                                *
 **********************************************************************/
```

* Use following template before function definition

```C++
/***********************************************************************
 * Name         : testMethod
 * Input        : int varName
 *              : int varOld
 * Ouput        : int varTest
 * Input/Output : int offset
 * Description  : This is a test function. It does many tests.
 **********************************************************************/
```

* Code format in a file

```C++
/**********************************************************************/
/***        Global Includes       E.g. <foo.h>                      ***/
/**********************************************************************/

/**********************************************************************/
/***        Local Includes        E.g. "bar.h"                      ***/
/**********************************************************************/

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Public Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Main routines, i.e. setup(), loop(), main()             ***/
/**********************************************************************/

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

/*
 * Deprecated or testing functions start here.
 */
```

* Use macro for printing debug messages

```C++
#define DEBUG

#ifdef DEBUG

    #define PRINT(...) Serial.print(__VA_ARGS__)
    #define PRINTF(...) Serial.printf(__VA_ARGS__)
    #define PRINTLN(...) Serial.println(__VA_ARGS__)
    #define PRINT_ARRAY(add, len) \
    do { \
        int debug_i_; \
        for (debug_i_ = 0 ; debug_i_ < (len) ; debug_i_++) { \
            Serial.printf("%02x", (unsigned int)((uint8_t*)(add))[debug_i_]); \
        } \
        Serial.println(); \
    } while(0)

#else /* DEBUG */

    #define PRINT(...)
    #define PRINTF(...)
    #define PRINTLN(...)
    #define PRINT_ARRAY(add, len)

#endif /* DEBUG */
```
Please see the `PRINTLN()` macro definition. If you write in the code `PRINTLN("Error not joining network")` and
compile the code with the `#define DEBUG` macro active, then the line `Error not joining network` will be printed. However, if you don't define the `DEBUG` macro, then it wont print anything.

Additionally, the `PRINT_ARRAY(address, len)` function simply prints in HEX an array passed as an argument.
E.g.

```C
uint8_t foo[255];

...

PRINT_ARRAY(foo, sizeof(foo))

this prints:

EF FF 01 AB ....
```

* Braces placement

```C++
if(pinMode == HIGH){
    ...
}
while(pinMode == HIGH){
    ...
}
void testMethod(){
    ...
}
```

* Always use braces form

```C++
// correct
if(pinMode == HIGH){
    pinMode = LOW;
}
// avoid
if(pinMode == HIGH) pinMode = LOW;
```
