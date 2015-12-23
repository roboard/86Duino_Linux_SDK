# 86Duino_Linux_SDK
Under L86duntu (based on Lubuntu 12.04), we can write programs with 86Duino Linux SDK (based on 86Duino Coding 210) to manipulate I/Os on 86Duino. In addition, there are many useful libraries such as *Ethernet*, *Wire* in this SDK.

## Language Reference
|                      |           API           |    Status   |
|:--------------------:|:-----------------------:|:-----------:|
|      Digital I/O     |        pinMode()        |      OK     |
|                      |      digitalWrite()     |      OK     |
|                      |      digitalRead()      |      OK     |
|      Analog I/O      |    analogReference()    | NO FUNCTION |
|                      |       analogRead()      |      OK     |
|                      |      analogWrite()      |      OK     |
|                      |  analogReadResolution() |      OK     |
|                      | analogWriteResolution() |      OK     |
|                      |     cpuTemperature()    |      OK     |
|     Advanced I/O     |          tone()         |      OK     |
|                      |         noTone()        |      OK     |
|                      |        shiftOut()       |      OK     |
|                      |        shiftIn()        |      OK     |
|                      |        pulseIn()        |      OK     |
|         Math         |          min()          |      OK     |
|                      |          max()          |      OK     |
|                      |          abs()          |      OK     |
|                      |       constrain()       |      OK     |
|                      |          map()          |      OK     |
|                      |          pow()          |      OK     |
|                      |          sqrt()         |      OK     |
|     Trigonometry     |          sin()          |      OK     |
|                      |          cos()          |      OK     |
|                      |          tan()          |      OK     |
|   Random   Numbers   |       randomSeed()      |      OK     |
|                      |         random()        |      OK     |
|   Bits and   Bytes   |        lowByte()        |      OK     |
|                      |        highByte()       |      OK     |
|                      |        bitRead()        |      OK     |
|                      |        bitWrite()       |      OK     |
|                      |         bitSet()        |      OK     |
|                      |        bitClear()       |      OK     |
|                      |          bit()          |      OK     |
| External   Interrupt |    attachInterrupt()    |      OK     |
|                      |    detachInterrupt()    |      OK     |
|      Interrupts      |       interrupts()      | NO FUNCTION |
|                      |      noInterrupts()     | NO FUNCTION |
|     Communication    |         Serial()        |      OK     |
|                      |       Serial232()       |      OK     |
|                      |       Serial485()       |      OK     |
|                      |         Stream()        |      OK     |

## Libraries
|       Library      | Support |  Status  |
|:------------------:|:-------:|:--------:|
|       EEPROM       |    X    |          |
|      Ethernet      |    O    |  Tested  |
|       Firmata      |    O    | Compiled |
|         GSM        |    O    |  Tested  |
|   LiquidCrystal    |    O    |  Tested  |
|         SD         |    X    |          |
|       Servo        |    X    |          |
|         SPI        |    O    |  Tested  |
|   SoftwareSerial   |    X    |          |
|       Stepper      |    O    | Compiled |
|         TFT        |    O    |  Tested  |
|        WiFi        |    X    |          |
|        Wire        |    O    |  Tested  |
|       Encoder      |    P    |  Tested  |
|       CANBus       |    X    |          |
|     Rosserial86    |    O    |  Tested  |
|       Servo86      |    X    |          |
|        Audio       |    X    |          |
|       USBHost      |    X    |          |
|      TimerOne      |    O    |  Tested  |
|      MsTimer2      |    X    |          |
|       Time86       |    X    |          |
|      FreeIMU1      |    O    | Compiled |
|       OneWire      |    O    | Compiled |
|  CapacitiveSensor  |    O    | Compiled |
|      Irremote      |    X    |          |
|     SpiderL3S      |    X    |          |
| Adafruit   CC3000  |    X    |          |
|   Adafruit Motor   |    O    | Compiled |
|     UTFT/Utouch    |    O    | Compiled |
|        GLCD        |    O    | Compiled |
|      LCD12864      |    O    | Compiled |
|       TLC5940      |    X    |          |
|        RF12        |    X    |          |
|        RF24        |    O    | Compiled |
|        Mirf        |    O    | Compiled |
|    VirtualWire     |    X    |          |
|      RadioHead     |    O    | Compiled |

* P => Partially Support
