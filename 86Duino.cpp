#include "Arduino.h"
#include "TimerOne.h"

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
	Timer1.initialize(100000);
	Timer1.pwm(9, 512);
	Timer1.attachInterrupt(blink);
    _time = millis();
}

void loop(void)
{
    digitalWrite(13, state);
    if( millis() - _time > 10000 )
    {
        Timer1.detachInterrupt();
        _time = millis();
    }
}
