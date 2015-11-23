#include "Arduino.h"

unsigned long _time;
int state = HIGH;
int ind = 2;

void blink()
{
    state = !state;
    //digitalWrite(13, state);
}

void setup(void) {
    pinMode(9, OUTPUT);
    pinMode(13, OUTPUT);
    analogWrite(9, 128);
    attachInterrupt(0, blink, CHANGE);
    attachInterrupt(1, blink, FALLING);
    attachInterrupt(2, blink, RISING);
    _time = millis();
}

void loop(void)
{
    digitalWrite(13, state);
    if( millis() - _time > 10000 )
    {
        detachInterrupt(ind--);
        _time = millis();
    }
}
