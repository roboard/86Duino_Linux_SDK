#include "Arduino.h"
#include "pthread.h"
#include "Wire.h"

pthread_t t1;
pthread_t t2;

void* i2c1(void* p) {
  int i = 0;
  char pVal[6] = {0};
  unsigned long pretime = 0L;
  while(1)
  {
	  Wire.beginTransmission(0x30 >> 1);
	  Wire.write(0xA8);
	  Wire.endTransmission();
	  
	  Wire.requestFrom((int)(0x30>>1), 6);
	  pretime = millis();
	  while ((millis() - pretime) < 1000UL) {
		if (Wire.available()) {
		  pVal[i++] = (uint8_t)Wire.read();
		  if (i == 6) break;
		  pretime = millis();
		}
	  }
	  i = 0;
	  printf("th1: %d %d %d %d %d %d\n", pVal[0],  pVal[1],  pVal[2],  pVal[3],  pVal[4],  pVal[5]);
	  //delay(300);
  }
}

void* i2c2(void* p) {
  int i = 0;
  char pVal[6] = {0};
  unsigned long pretime = 0L;
  while(1)
  {
	  Wire.beginTransmission(0xD4 >> 1);
	  Wire.write(0xA8);
	  Wire.endTransmission();
	  
	  Wire.requestFrom((int)(0xD4>>1), 6);
	  pretime = millis();
	  while ((millis() - pretime) < 1000UL) {
		if (Wire.available()) {
		  pVal[i++] = (uint8_t)Wire.read();
		  if (i == 6) break;
		  pretime = millis();
		}
	  }
	  i = 0;
	  printf("th2: %d %d %d %d %d %d\n", pVal[0],  pVal[1],  pVal[2],  pVal[3],  pVal[4],  pVal[5]);
	  //delay(300);
  }
}
void setup() {
  int err;
  Wire.begin();
  err = pthread_create(&t1, NULL, i2c1, NULL);
  if(err != 0) printf("Create thread 1 fail\n");
  err = pthread_create(&t2, NULL, i2c2, NULL);
  if(err != 0) printf("Create thread 2 fail\n");
}


void loop() {
}
