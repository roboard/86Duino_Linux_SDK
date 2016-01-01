#### v1.00 ####

For 86Duino Standard APIs *see [86Duino Language Reference page](http://www.86duino.com/?page_id=2255)*,
you can call them like Arduino except interrupts() and noInterrupts() that are invalid under Linux.

_Note_: In Linux 86Duino Standard APIs, the USB Serial is replaced by Hardware Serial1 (COM1), the data will be send from COM1 if you call like Serial.write() or Serial.print() function in your program. 

In addition, 86Duino Linux SDK also support the following libraries:
 
* Ethernet
* Firmata
* GSM
* LiquidCrystal 
* SPI
* Stepper
* TFT
* Wire
  - In Linux SDK, four addditional funtions are introduced for use in case of multi-thread programming: send(), receive(), sensorRead(), sensorReadEX()
* Encoder
  - _NOTE_: In Linux SDK, not every function is implemented; some functions become invalid in Linux SDK, ex: setIndexReset(), setComparator(), setRange(), readNanoseconds(), attachInterrupt(), detachInterrupt() 
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