#include "Arduino.h"
#include <stdio.h>
#include <stdlib.h>

// add 3rd party lib in here
#include "TimerOne.h"

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

const int led = LED_BUILTIN;  // the pin with a LED

// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables
OSSPIN exspin;

void blinkLED(void)
{
  OSSPINLOCK(exspin);
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  OSSPINUNLOCK(exspin);
  digitalWrite(led, ledState);
}

int main()
{
  unsigned long blinkCopy = 0;  // holds a copy of the blinkCount

  init();
  interrupt_init();
  OSSPININIT(exspin);

  pinMode(led, OUTPUT);
  Timer1.initialize(150000);
  Timer1.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds

  while(blinkCopy < 10)
  {
          // to read a variable which the interrupt code writes, we
          // must temporarily disable interrupts, to be sure it will
          // not change while we are reading.  To minimize the time
          // with interrupts off, just quickly make a copy, and then
          // use the copy while allowing the interrupt to keep working.
          OSSPINLOCK(exspin);
          blinkCopy = blinkCount;
          OSSPINUNLOCK(exspin);

          printf("blinkCount = %lu\n", blinkCopy);
          delay(100);
  }

  Timer1.stop();

  return 0;
}

