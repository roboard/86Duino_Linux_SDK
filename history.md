#### v1.00 ####

For 86Duino Standard APIs *see [86Duino Language Reference page](http://www.86duino.com/?page_id=2255)*,
you can call them like Arduino except interrupts() and noInterrupts() that are invalid under Linux.

In addition, 86Duino Linux SDK also support the following libraries:
 
* Ethernet
* Firmata
* GSM
* LiquidCrystal 
* SPI
* Stepper
* TFT
* Wire
  - Add four funtions for multi-thread condition: send(), receive(), sensorRead(), sensorReadEX()
* Encoder
  - _NOTE_: Not all functions are implemented in this library.
          Some functions are unused, ex: setIndexReset(), setComparator(), setRange(),
		  readNanoseconds(), attachInterrupt(), detachInterrupt() 
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