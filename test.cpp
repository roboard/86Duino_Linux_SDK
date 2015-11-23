#include "Arduino.h"
#include "pthread.h"

unsigned long _time;
void* test_thread1(void* p) {

}

pthread_spinlock_t asd;
void setup(void) {
	init();
	pinMode(9, OUTPUT);
	pinMode(10, INPUT_PULLUP);
	digitalWrite(9, HIGH);
	digitalRead(10);
	_time = millis();
        unsigned int  aa = random();
	lowByte(0x9999);
	pthread_t aaa;
	pthread_create(&aaa, NULL, test_thread1, NULL);

	pthread_spin_init(&asd, NULL);
	lockGPIO(0);
}

void loop(void) {
    printf("%ld\r", millis() - _time);
    delay(100);
}
