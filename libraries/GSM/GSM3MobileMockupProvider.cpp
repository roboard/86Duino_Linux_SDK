/*
This file is part of the GSM3 communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telefónica Digital - PDI -
- Physical Internet Lab, as part as its collaboration with
Arduino and the Open Hardware Community. 

September-December 2012

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

The latest version of this library can always be found at
https://github.com/BlueVia/Official-Arduino
*/
#include <GSM3MobileNetworkProvider.h>
#include <GSM3MobileMockupProvider.h>
#include <inttypes.h>
#include <HardwareSerial.h>


GSM3MobileMockupProvider::GSM3MobileMockupProvider()
{   
	lineStatus=IDLE;
	msgExample="Hello#World";
	msgIndex=0;
};

int GSM3MobileMockupProvider::begin(char* pin)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::begin()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::begin()\n");
#endif
	return 0;
};

int GSM3MobileMockupProvider::ready()
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::ready()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::ready()\n");
#endif
	return 1;
};

int GSM3MobileMockupProvider::beginSMS(const char* number)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("SM3MobileMockupProvider::beginSMS()");
#elif defined (DMP_LINUX)
	printf("SM3MobileMockupProvider::beginSMS()\n");
#endif
	return 0;
};

void GSM3MobileMockupProvider::writeSMS(char c)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.print(c);
#elif defined (DMP_LINUX)
	printf("%c", c);
#endif
};

int GSM3MobileMockupProvider::endSMS()
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::endSMS()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::endSMS()\n");
#endif
};

int GSM3MobileMockupProvider::availableSMS()
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::availableSMS()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::availableSMS()\n");
#endif

	return 120;
};

int GSM3MobileMockupProvider::peek()
{
	return (int)'H';
};

int GSM3MobileMockupProvider::remoteSMSNumber(char* number, int nlength)
{
	if(nlength>=13)
		strcpy(number, "+34630538546");
	return 12;
};


void GSM3MobileMockupProvider::flushSMS()
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::flushSMS()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::flushSMS()\n");
#endif
};

int GSM3MobileMockupProvider::readSMS()
{
	if(msgExample[msgIndex]==0)
	{
		msgIndex=0;
		return 0;
	}
	else
	{
		msgIndex++;
		return msgExample[msgIndex-1];
	};
};

int GSM3MobileMockupProvider::connectTCPClient(const char* server, int port, int id_socket)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::connectTCPClient()");
	Serial.print(server);Serial.print(":");Serial.print(port);Serial.print("-");Serial.println(id_socket);
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::connectTCPClient()\n");
	printf("%s", server);printf(":");printf("%d", port);printf("-");printf("%d\n", id_socket);
#endif
}

void GSM3MobileMockupProvider::writeSocket(const uint8_t *buf, size_t size, int id_socket)
{
	int i;
	for(i=0;i<size;i++)
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
		Serial.print(buf[i]);
#elif defined (DMP_LINUX)
		printf("%d", buf[i]);
#endif
}
/* I'm taking this off. We'll reply from the NetworkProvider
uint8_t GSM3MobileMockupProvider::getStatus(uint8_t socket)
{
    if((socket>=minSocket())&&(socket<=maxSocket()))
		return 1;
	else
		return 0;
};
*/

int GSM3MobileMockupProvider::readSocket(uint8_t *buf, size_t size, int idsocket)
{
	int i;
	int l=strlen(msgExample);
	for(i=0;(i<size)&&(i<l);i++)
		buf[i]=msgExample[i];
	buf[i]=0;
	return i;
}

int GSM3MobileMockupProvider::availableSocket(int idsocket)
{
	return 1;
};

int GSM3MobileMockupProvider::readSocket(int idsocket, bool advance)
{
	char c;
	if(msgExample[msgIndex]==0)
	{
		msgIndex=0;
		return 0;
	}
	else
	{
		c=msgExample[msgIndex];
		if(advance)
			msgIndex++;
	};
	return c;
};

void GSM3MobileMockupProvider::flushSocket(int idsocket)
{
	while(readSocket(idsocket));
};

int GSM3MobileMockupProvider::disconnectTCP(int idsocket)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::disconnectTCP()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::disconnectTCP()\n");
#endif
	return 1;
};

int GSM3MobileMockupProvider::connectTCPServer(int port, char* localIP, int* localIPlength)
{
#if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP)
	Serial.println("GSM3MobileMockupProvider::connectTCPServer()");
#elif defined (DMP_LINUX)
	printf("GSM3MobileMockupProvider::connectTCPServer()\n");
#endif
	if((localIP!=0)&&(*localIPlength>12))
		strcpy("192.168.1.1", localIP);
	return 1;
};

bool GSM3MobileMockupProvider::getSocketModemStatus(uint8_t s)
{
	// Feeling lazy
	return true;
}

