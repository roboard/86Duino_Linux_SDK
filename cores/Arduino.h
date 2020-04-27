#ifndef Arduino_h
#define Arduino_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <stdbool.h>
#include "binary.h"

// linux
#include <stdint.h>
#include <unistd.h>
//#include "itoa.h"

#include "avr/pgmspace.h"
#include "v86clock.h"
#include "io.h"
#include "pins_arduino.h"

#include "OSAbstract.h"

#ifdef __cplusplus
extern "C"{
#endif

// void yield(void);

#define LOW          (0x00)
#define HIGH         (0x01)
#define CHANGE       (0x02)
#define FALLING      (0x03)
#define RISING       (0x04)

#define INPUT          (0x00)
#define OUTPUT         (0x01)
#define INPUT_PULLUP   (0x02)
#define INPUT_PULLDOWN (0x03)

#define PCSPEAKER    (250)

#if defined (DMP_LINUX)
  #define PI 3.1415926535897932384626433832795
#endif
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define SERIAL   (0x00)
#define DISPLAY  (0x01)

#define LSBFIRST (0)
#define MSBFIRST (1)

#define INTERNAL 3
#define DEFAULT 1
#define EXTERNAL 0

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#define min(a,b)                ((a)<(b)?(a):(b))
#define max(a,b)                ((a)>(b)?(a):(b))
//#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)                ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)            ((deg)*DEG_TO_RAD)
#define degrees(rad)            ((rad)*RAD_TO_DEG)
#define sq(x)                   ((x)*(x))

#define interrupts()   io_EnableINT()
#define noInterrupts() io_DisableINT()

#define F_CPU                        (300000000L)
#define clockCyclesPerMicrosecond()  (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a) ((a) * clockCyclesPerMicrosecond())

#define lowByte(w)  ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// for 86Duino
#define SB_CROSSBASE  (0x64)
#define CROSSBARBASE  (0x0A00)
#define SB_FCREG      (0xC0)
#define SB_GPIOBASE   (0x62)
#define GPIOCTRLBASE  (0xF100)
#define GPIODATABASE  (0xF200)
#define GPIODIRBASE   (0xF202)

typedef unsigned short word;

#define bit(b)    (1UL << (b))
#define DEGREE_C    (0)
#define DEGREE_F    (1)

typedef bool boolean;
typedef uint8_t byte;

bool init(void);
void initVariant(void);

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int  digitalRead(uint8_t);
int  analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, uint32_t);
void analogReadResolution(int res);
void analogWriteResolution(int res);
void Close_Pwm(uint8_t);

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long);
void delayMicroseconds(unsigned long us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

int interrupt_init(void);
void attachInterrupt(uint8_t, void (*userFunc)(void), int mode);
void detachInterrupt(uint8_t);
void attachTimerInterrupt(uint8_t, void (*callback)(void), uint32_t);
void detachTimerInterrupt(void);

#if defined (DMP_DOS_BS) || defined (DMP_DOS_DJGPP)
// wdt1 functions
extern bool rebootByWDT;
void wdt_enable(unsigned long nTime);
void wdt_disable(void);
void wdt_reset(void);
void wdt_init(void);
bool get_wdt_timeout(void);
#endif

void setup(void);
void loop(void);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

double cpuTemperature(void);
double cpuTemperature(uint8_t);

//extern void* USBDEV;
extern bool Global_irq_Init;
//extern bool timer1_pin32_isUsed; // defined in tone.cpp
extern unsigned long CLOCKS_PER_MICROSEC;
extern unsigned long VORTEX86EX_CLOCKS_PER_MS;
#define Serial      Serial1
#define Serial4    Serial485
#define Serial6    Serial232
#include "WCharacter.h"
#include "WString.h"
#include "HardwareSerial.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);

#endif

#endif
