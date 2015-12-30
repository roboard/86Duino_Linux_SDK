#86Duino Linux SDK v1.00

INTRODUCTION
------------

Under L86duntu (based on Lubuntu 12.04), we can write programs with
86Duino Linux SDK (based on 86Duino Coding 210) to manipulate I/Os on 86Duino.
The user can include "Arduino.h" to call all 86Duino API (ex. digitalWrite)
listed in the [86Duino Language Reference](http://www.86duino.com/?page_id=2255).

In 86Duino Linux SDK also support multi-thread for all 86Duino API and
the following libraries:

* Ethernet
* Firmata
* GSM
* LiquidCrystal 
* SPI
* Stepper
* TFT
* Wire
* Encoder
* Rosserial86
* TimerOne
* FreeIMU1
* Adafruit Motor
* GLCD
* LCD12864
* RF12
* RF24
* Mirf
* RadioHead

HOW TO COMPILE
--------------

Copy the Makefile in the example folder to your program folder 
that you want to compile, and then type "make" command to compile source
code to produce your 86Duino linux program (if you want to use 86Duino libraries,
see the following "USE THIRD-PARTY LIBRARIES" section).


USE THIRD-PARTY LIBRARIES
-------------------------

If you need to use in your program 86Duino libraries listed in the [86Duino 
Libraries Reference](http://www.86duino.com/?page_id=2257), please modify
the Makefile as follows:

  Add the names of the libraries that you want to use behind the 
  "THIRD_LIB_NAME" variable (ex. to add the TimerOne library to compile, you 
  need to modify like "THIRD_LIB_NAME := TimerOne" in Makefile). Since some 
  libraries may depend on other libraries, the names of those libraries must 
  also be added behind the variable "THIRD_LIB_NAME". See APPENDIX.1 for a
  list that describes the relation of libraries.

#### APPENDIX 1 ####

* FreeIMU1:         Wire
* Mirf:             SPI
* RadioHead:        SPI
* RF24:             SPI
* TFT:              SPI
