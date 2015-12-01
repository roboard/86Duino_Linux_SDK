/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 01 November 2013 by Android Lin
*/

#define __HARDWARE_SERIAL_LIB

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"

#include "HardwareSerial.h"

#define  COM1_TX    (0x9A)
#define  COM1_RX    (0x9B)
#define  COM2_TX    (0x9E)
#define  COM2_RX    (0x9F)
#define  COM3_TX    (0x9C)
#define  COM3_RX    (0x9D)

// Public Methods //////////////////////////////////////////////////////////////
HardwareSerial::HardwareSerial(int com_port, unsigned long com_baudrate, unsigned char com_format, unsigned long com_rxtimeout, unsigned long com_txtimeout) {
	port        = com_port;
	baudrate    = com_baudrate;
	format      = com_format;
	rxtimeout   = com_rxtimeout;
	txtimeout   = com_txtimeout;
	peek_stored = false;
	hadbegin    = false;
}


void HardwareSerial::begin(unsigned long baud) {
	begin(baud, format, COM_FullDuplex);
}

void HardwareSerial::begin(unsigned long baud, int comtype) {
	begin(baud, format, comtype);
}

void HardwareSerial::begin(unsigned long baud, uint8_t config) {
	begin(baud, config, COM_FullDuplex);
}

void HardwareSerial::begin(unsigned long baud, uint8_t config, int comtype) {
    unsigned short crossbar_ioaddr = 0;
	if(hadbegin == true) return;

	if (io_Init() == false) {
		printf("ERROR: IO init fail.\n");
		return;
	}
	sb_Write(0xc0, sb_Read(0xc0) & 0x7fffffffL | ((unsigned long)1L << 31));
	io_Close();

	if (com_Init(port) == false)
	{
		printf("COM init fail!!\n");
		return;
	}

	switch(baud)
	{
	case 748800L:  baud = COMBAUD_748800BPS;  break;
	case 499200L:  baud = COMBAUD_499200BPS;  break;
	case 249600L:  baud = COMBAUD_249600BPS;  break;
	case 115200L:  baud = COMBAUD_115200BPS;  break;
	case 57600L:  baud = COMBAUD_57600BPS;  break;
	case 38400L:  baud = COMBAUD_38400BPS;  break;
	case 19200L:  baud = COMBAUD_19200BPS;  break;
	case 9600L:  baud = COMBAUD_9600BPS;  break;
	case 4800L:  baud = COMBAUD_4800BPS;  break;
	case 2400L:  baud = COMBAUD_2400BPS;  break;
	case 1200L:  baud = COMBAUD_1200BPS;  break;
	case 300L:  baud = COMBAUD_300BPS;  break;
	case 50L:  baud = COMBAUD_50BPS;  break;
	default: baud = COMBAUD_9600BPS; break;
	}

	com_SetBaud(port, baud);
	com_SetFormat(port, config);
	com_ClearWFIFO(port);
	com_ClearRFIFO(port);
	//com_SetTimeOut(handle, rxtimeout, txtimeout);
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	
	#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE)
	if(port == COM1 || port == COM2 || port == COM3)
		if(comtype == COM_HalfDuplex) com_EnableHalfDuplex(port);
	#endif
	
	if(port == COM1)
	{
		io_outpb(crossbar_ioaddr + COM1_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM1_RX, 0x08);
	}
	else if(port == COM2)
	{
		io_outpb(crossbar_ioaddr + COM2_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM2_RX, 0x08);
	}
	else if(port == COM3)
	{
		io_outpb(crossbar_ioaddr + COM3_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM3_RX, 0x08);
	}
    hadbegin = true;
}

void HardwareSerial::end() {
	if(hadbegin == false) return;
	com_FlushWFIFO(port);
	com_Close(port);
	hadbegin = false;
}

int HardwareSerial::available(void) {
    if(hadbegin == false) return 0;
	return com_QueryRFIFO(port);
} 

int HardwareSerial::peek(void) {
  	if(hadbegin == false) return -1;
	if(peek_stored == true)
  		return peek_val;
  	else
  	{
  		if((peek_val = com_Read(port)) == 0xFFFF)
  			return -1;//peek_val = -1;
  		peek_stored = true;
		return peek_val;
	}
}

int HardwareSerial::read(void) {
	int c;
	if(hadbegin == false) return -1;
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
		c = com_Read(port);
		return (c == 0xFFFF) ? -1 : c;
	}
}

void HardwareSerial::flush() {
    if(hadbegin == false) return;
	com_FlushWFIFO(port);
}

size_t HardwareSerial::write(uint8_t c) {
	if(hadbegin == false) return 0;
	return (com_Write(port, c) == true) ? 1 : 0;
} 

HardwareSerial::operator bool() {
	return true;
} 

void serialEvent() __attribute__((weak));
void serialEvent1() __attribute__((weak));
void serialEvent2() __attribute__((weak));
void serialEvent3() __attribute__((weak));
void serialEvent485() __attribute__((weak));
//void serialEvent232() __attribute__((weak));
void serialEvent() {}
void serialEvent1() {}
void serialEvent2() {}
void serialEvent3() {}
void serialEvent485() {}
//void serialEvent232() {}
void serialEventRun(void)
{
//	if(USBDEV != NULL && Serial.available() > 0) serialEvent();
	if(Serial1.hadbegin == true && Serial1.available() > 0) serialEvent1();
	if(Serial2.hadbegin == true && Serial2.available() > 0) serialEvent2();
	if(Serial3.hadbegin == true && Serial3.available() > 0) serialEvent3();
	if(Serial485.hadbegin == true && Serial485.available() > 0) serialEvent485();
//	if(Serial232.hadbegin == true && Serial232.available() > 0) serialEvent232();
}

HardwareSerial Serial1(COM1, 115200L, BYTESIZE8|COM_NOPARITY|STOPBIT1, 0L, 500L);
HardwareSerial Serial2(COM2, 115200L, BYTESIZE8|COM_NOPARITY|STOPBIT1, 0L, 500L);
HardwareSerial Serial3(COM3, 115200L, BYTESIZE8|COM_NOPARITY|STOPBIT1, 0L, 500L);
HardwareSerial Serial485(COM4, 115200L, BYTESIZE8|COM_NOPARITY|STOPBIT1, 0L, 500L);
//HardwareSerial Serial232(COM6, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 0L, 500L);
HardwareSerial* HWSerial[4] = {NULL, &Serial1, &Serial2, &Serial3};
// Preinstantiate Objects //////////////////////////////////////////////////////
