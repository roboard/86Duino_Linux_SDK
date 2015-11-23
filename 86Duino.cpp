#include "Arduino.h"

unsigned long _time;
int state = LOW;
int ind = 128;

void blink()
{
    state = !state;
}

void setup(void) {
    pinMode(9, OUTPUT);
    pinMode(13, OUTPUT);
    analogWrite(9, 128);
    //attachInterrupt(0, blink, CHANGE);
    //attachInterrupt(1, blink, FALLING);
    //attachInterrupt(2, blink, RISING);
	attachTimerInterrupt(128, blink, 500000);
    _time = millis();
}

void loop(void)
{
    digitalWrite(13, state);
    /*if( millis() - _time > 10000 )
    {
        detachInterrupt(ind--);
        _time = millis();
    }*/
}
