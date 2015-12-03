/*
  com.h - DM&P Vortex86 Universal Serial library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef __DMP_COM_H
#define __DMP_COM_H

#define COM_LIB_TIMEOUT_DEBUG    (0)

#include "dmpcfg.h"
#include "uart.h"
#include "USBCore.h"
#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef struct
{
	int   com;
	void *func;
	
	DMPAPI(void) (*Close)(void *);
	DMPAPI(bool) (*SetBPS)(void *, unsigned long); // only for UART & CAN
	DMPAPI(void) (*SetTimeOut)(void *, unsigned long, unsigned long);
	
	DMPAPI(unsigned int) (*Read)(void *);
	DMPAPI(int)  (*Receive)(void *, unsigned char *, int);
	DMPAPI(int)  (*QueryRxQueue)(void *);
	DMPAPI(bool) (*RxQueueFull)(void *);
	DMPAPI(bool) (*RxQueueEmpty)(void *);
	DMPAPI(void) (*FlushRxQueue)(void *);

	DMPAPI(bool) (*Write)(void *, unsigned char);
	DMPAPI(int)  (*Send)(void *, unsigned char *, int);
	DMPAPI(int)  (*QueryTxQueue)(void *);
	DMPAPI(bool) (*TxQueueFull)(void *);
	DMPAPI(bool) (*TxQueueEmpty)(void *);
	DMPAPI(void) (*FlushTxQueue)(void *);
	DMPAPI(bool) (*TxReady)(void *);
	DMPAPI(void) (*FlushWFIFO)(void *);
	
	DMPAPI(void) (*SetFormat)(void *, unsigned char);
	DMPAPI(void) (*SetFlowControl)(void *, int);
	DMPAPI(void) (*EnableFIFO)(void *, int);
	DMPAPI(bool) (*SetWFIFOSize)(void *, int);
	DMPAPI(void) (*ClearRFIFO)(void *);
	DMPAPI(void) (*ClearWFIFO)(void *);
	DMPAPI(void) (*SetLSRHandler)(void *, void (*)(SerialPort *));
	DMPAPI(void) (*SetMSRHandler)(void *, void (*)(SerialPort *));
	DMPAPI(unsigned char) (*GetLSR)(void *);
	DMPAPI(unsigned char) (*GetMSR)(void *);
	DMPAPI(void) (*EnableHalfDuplex)(void *);
	DMPAPI(void) (*EnableFullDuplex)(void *);
	DMPAPI(void) (*EnableDebugMode)(void *);
	DMPAPI(void) (*DisableDebugMode)(void *);
	
} COMPort;
*/

#define SIZE_OF_COM		(12) // (COM1 ~ COM10) + USB_COM + CAN_BUS

#define COM1			   	(0x00)
#define COM2			   	(0x01)
#define COM3			   	(0x02)
#define COM4			   	(0x03)
/*
#define COM5			   	(0x04)
#define COM6			   	(0x05)
#define COM7			   	(0x06)
#define COM8			   	(0x07)
#define COM9			   	(0x08)
#define COM10				(0x09)
#define USB_COM		                    (0x0A)
#define CAN_BUS				(0x0B)
*/
DMPAPI(bool) com_Init(int com);
     #define COM_ADUPLEX            (0xff)  // auto. select duplex mode according to RoBoard's version
     #define COM_FDUPLEX            (0)     // full-duplex mode
     #define COM_HDUPLEX            (1)     // half-duplex by TX/RX-short
     #define COM_HDUPLEX_RTS        (2)     // half-duplex by RTS-control
     #define COM_HDUPLEX_TXDEN      (3)     // half-duplex by TXDEN-control
DMPAPI(void) com_Close(int com);

DMPAPI(bool) com_SetBaud(int com, unsigned int bps);
    // for UART
    //#define COMBAUD_748800BPS       (0x8002)  // 57600 * 13 (invalid for RB-100)
    //#define COMBAUD_499200BPS       (0x8003)  // 38400 * 13 (invalid for RB-100)
    //#define COMBAUD_249600BPS       (0x8006)  // 19200 * 13 (invalid for RB-100)
    #define COMBAUD_115200BPS       (0x0001)
    #define COMBAUD_57600BPS        (0x0002)
    #define COMBAUD_38400BPS        (0x0003)
    #define COMBAUD_19200BPS        (0x0006)
    #define COMBAUD_9600BPS         (0x000c)
    #define COMBAUD_4800BPS         (0x0018)
    #define COMBAUD_2400BPS         (0x0030)
    #define COMBAUD_1200BPS         (0x0060)
    #define COMBAUD_300BPS          (0x0180)
    #define COMBAUD_50BPS           (0x0900)

//DMPAPI(void) com_SetTimeOut(int com, unsigned long rx_timeout, unsigned long tx_timeout);
#define NO_TIMEOUT         (-1)

DMPAPI(unsigned int) com_Read(int com); // not for CAN bus
DMPAPI(bool)  com_Receive(int com, unsigned char* buf, int bsize);
DMPAPI(int) com_QueryRFIFO(int com);
/*
DMPAPI(int)  com_QueryRxQueue(COMPort *port);
DMPAPI(bool) com_RxQueueFull(COMPort *port);
DMPAPI(bool) com_RxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushRxQueue(COMPort *port);
*/

DMPAPI(bool) com_Write(int com, unsigned char val); // not for CAN bus
DMPAPI(bool) com_Send(int com, unsigned char* buf, int bsize);
DMPAPI(bool) com_FlushWFIFO(int com);

/*
DMPAPI(int)  com_QueryTxQueue(COMPort *port);
DMPAPI(bool) com_TxQueueFull(COMPort *port);
DMPAPI(bool) com_TxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushTxQueue(COMPort *port);
DMPAPI(bool) com_TxReady(COMPort *port);
*/

/* only for UART */
DMPAPI(bool) com_SetFormat(int com, unsigned char format);
#define BYTESIZE5          (0x00)
#define BYTESIZE6          (0x01)
#define BYTESIZE7          (0x02)
#define BYTESIZE8          (0x03)
#define STOPBIT1           (0x00)
#define STOPBIT2           (0x04)
#define COM_NOPARITY           (0x00)
#define COM_ODDPARITY          (0x08)
#define COM_EVENPARITY         (0x18)

//-- values for the "bytesize" argument
#define COM_BYTESIZE5          (5)
#define COM_BYTESIZE6          (6)
#define COM_BYTESIZE7          (7)
#define COM_BYTESIZE8          (8)

/*
DMPAPI(void) com_SetFlowControl(COMPort *port, int control);
#define NO_CONTROL         (0)
#define RTS_CTS            (1)
#define XON_XOFF           (2)
DMPAPI(void) com_EnableFIFO(COMPort *port, int fifo);
#define FIFO_001		   (1)
#define FIFO_016		   (16)
#define FIFO_032		   (32)
#define FIFO_128		   (128)
DMPAPI(bool) com_SetWFIFOSize(COMPort *port, int size); // setup after enable/disable fifo

DMPAPI(void) com_SetLSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(void) com_SetMSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(unsigned char) com_GetLSR(COMPort *port);
#define LSR_EB             (0x80)
#define LSR_TEMT           (0x40)
#define LSR_THRE           (0x20)
#define LSR_BI             (0x10)
#define LSR_FE             (0x08)
#define LSR_PE             (0x04)
#define LSR_OE             (0x02)
#define LSR_DR             (0x01)
DMPAPI(unsigned char) com_GetMSR(COMPort *port);
#define MSR_DCD            (0x80)
#define MSR_RI             (0x40)
#define MSR_DSR            (0x20)
#define MSR_CTS            (0x10)
#define MSR_DDCD           (0x08)
#define MSR_TERI           (0x04)
#define MSR_DDSR           (0x02)
#define MSR_DCTS           (0x01)
*/
DMPAPI(bool) com_ClearRFIFO(int com);
DMPAPI(bool) com_ClearWFIFO(int com);
DMPAPI(void) com_EnableHalfDuplex(int com);
DMPAPI(void) com_EnableFullDuplex(int com);
//DMPAPI(void) com_EnableDebugMode(COMPort *port);
//DMPAPI(void) com_DisableDebugMode(COMPort *port);

DMPAPI(void) com_EnableTurboMode(int com);
DMPAPI(void) com_DisableTurboMode(int com);
DMPAPI(bool) com_IsTurboMode(int com);
DMPAPI(void) com_EnableFIFO32(int com);
DMPAPI(void) com_DisableFIFO32(int com);
DMPAPI(bool) com_IsFIFO32Mode(int com);

#ifdef __cplusplus
}
#endif

#endif
