#include "Arduino.h"
#include "Wire.h"

void setup() {
  Wire.begin();
printf("%ld\n", vx86_CpuCLK());
}

int i = 0;
unsigned long pretime = 0L;
static char pVal[6] = {0};
void loop() {

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
  printf("%d %d %d %d %d %d\n", pVal[0],  pVal[1],  pVal[2],  pVal[3],  pVal[4],  pVal[5]);
  delay(300);
}
