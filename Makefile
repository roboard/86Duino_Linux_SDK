TARGETS   = libcore.a libSPI.a libWire.a libAdafruit_Motor_Shield.a libEncoder.a libEthernet.a libFirmata.a \
            libFreeIMU1.a libGLCD.a libGSM.a libLCD12864.a libLiquidCrystal.a libMirf.a \
            libRadioHead.a libRF12.a libRF24.a libRosserial86.a libStepper.a libTFT.a libTimerOne.a

CTARGETS  = ccore cSPI cWire cAdafruit_Motor_Shield cEncoder cEthernet cFirmata \
            cFreeIMU1 cGLCD cGSM cLCD12864 cLiquidCrystal cMirf cRadioHead cRF12 \
            cRF24 cRosserial86 cStepper cTFT cTimerOne

.PHONY : everything all clean

everything : $(TARGETS)

all : clean everything

clean : $(CTARGETS)

libcore.a:
	make -C cores

libSPI.a:
	make -C libraries/SPI

libWire.a:
	make -C libraries/Wire

libAdafruit_Motor_Shield.a:
	make -C libraries/Adafruit_Motor_Shield

libEncoder.a:
	make -C libraries/Encoder

libEthernet.a:
	make -C libraries/Ethernet

libFirmata.a:
	make -C libraries/Firmata

libFreeIMU1.a:
	make -C libraries/FreeIMU1

libGLCD.a:
	make -C libraries/GLCD
	
libGSM.a:
	make -C libraries/GSM

libLCD12864.a:
	make -C libraries/LCD12864

libLiquidCrystal.a:
	make -C libraries/LiquidCrystal

libMirf.a:
	make -C libraries/Mirf

libRadioHead.a:
	make -C libraries/RadioHead

libRF12.a:
	make -C libraries/RF12

libRF24.a:
	make -C libraries/RF24

libRosserial86.a:
	make -C libraries/Rosserial86

libStepper.a:
	make -C libraries/Stepper

libTFT.a:
	make -C libraries/TFT

libTimerOne.a:
	make -C libraries/TimerOne

# remove files
ccore:
	make clean -C cores

cSPI:
	make clean -C libraries/SPI

cWire:
	make clean -C libraries/Wire

cAdafruit_Motor_Shield:
	make clean -C libraries/Adafruit_Motor_Shield

cEncoder:
	make clean -C libraries/Encoder

cEthernet:
	make clean -C libraries/Ethernet

cFirmata:
	make clean -C libraries/Firmata

cFreeIMU1:
	make clean -C libraries/FreeIMU1

cGLCD:
	make clean -C libraries/GLCD
	
cGSM:
	make clean -C libraries/GSM

cLCD12864:
	make clean -C libraries/LCD12864

cLiquidCrystal:
	make clean -C libraries/LiquidCrystal

cMirf:
	make clean -C libraries/Mirf

cRadioHead:
	make clean -C libraries/RadioHead

cRF12:
	make clean -C libraries/RF12

cRF24:
	make clean -C libraries/RF24

cRosserial86:
	make clean -C libraries/Rosserial86

cStepper:
	make clean -C libraries/Stepper

cTFT:
	make clean -C libraries/TFT

cTimerOne:
	make clean -C libraries/TimerOne