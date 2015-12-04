/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.c/socket.h
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 *
 * Modified by Denny Yang on 25 Sep 2013:
 *    - making to compile in x86 for Intel Galileo Arduino IDE
 */

#include "Ethernet.h"
#include "Udp.h"
#include "Dns.h"
#include "EthernetUdp.h"
 
#if defined (DMP_LINUX)
#include <errno.h>		// -EINVAL, -ENODEV
#include <netdb.h>		// gethostbyname
#include <sys/poll.h>
#include <sys/types.h>		// connect
#include <sys/socket.h>		// connect
#include <sys/ioctl.h>		// ioctl
#endif

#define MY_TRACE_PREFIX "EthernetUDP"

/* Constructor */
EthernetUDP::EthernetUDP()
{
#if defined (DMP_LINUX)
	_sock = -1;
#elif defined (DMP_DOS_DJGPP)
	TxSize = 0;
	RxHead = 0;
	RxTail = 0;
	RxSize = 0;
	sws = (struct SwsSockInfo*)malloc(sizeof(struct SwsSockInfo));
	if (sws) {
		memset(sws, 0, sizeof(struct SwsSockInfo));
		sws->_sock = SWS_INVALID_SOCKET;
	}
#endif
}

#if defined (DMP_LINUX)
int EthernetUDP::listen()
{
	bzero(&_sin,sizeof(_sin));
	_sin.sin_family = AF_INET;
	_sin.sin_addr.s_addr=htonl(INADDR_ANY);
	_sin.sin_port=htons(_port);

	return bind(_sock,(struct sockaddr *)&_sin,sizeof(_sin));
}
#elif defined (DMP_DOS_DJGPP)
EthernetUDP::~EthernetUDP()
{
	if (sws) {
		free(sws);
		sws = NULL;
	}
}
#endif

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDP::begin(uint16_t port)
{
#if defined (DMP_LINUX)
	_port = port;
	_remaining = 0;
	int ret = socket(AF_INET, SOCK_DGRAM, 0);
	if ( ret < 0){
		return 0;
	}

	_sock = ret;
	if (listen() != 0){
		close(_sock);
		return 0;
	}

	int on=1;
	ret = ioctl(_sock, FIONBIO, (char *)&on);
	if (ret < 0) {
		close(_sock);
		return 0;
	}

	return 1;
#elif defined (DMP_DOS_DJGPP)
	struct SWS_sockaddr_in sin;
	SWS_u_long lArg = 1;
	int yes = 1, no = 0;
	
	if (sws == NULL) return 0;
	
	if (sws->_sock != SWS_INVALID_SOCKET)
		return 0;
	
	sws->_sock = SWS_socket(SWS_AF_INET, SWS_SOCK_DGRAM, 0);
    if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	bzero(&sin, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = SwsSock.getULLocalIp();
    sin.sin_port = SWS_htons(port);
	
	if (SWS_bind(sws->_sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
    }
	
	if (SWS_ioctl(sws->_sock, SWS_FIONBIO, &lArg) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&no, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}
	
	_port = port;
	TxSize = 0;
	RxHead = 0;
	RxTail = 0;
	RxSize = 0;
	
	return 1;
#endif
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int EthernetUDP::available() {
#if defined (DMP_LINUX)
  	struct pollfd ufds;
	int ret = 0;
	extern int errno;
	int    timeout = 0;	// milliseconds

	if (_sock == -1)
		return 0;

	ufds.fd = _sock;
	ufds.events = POLLIN;
	ufds.revents = 0;

	ret = poll(&ufds, 1, timeout);
	if ( ret < 0 ){
		return 0;
	}
	if( ret == 0)
		return 0;

	// only return available if bytes are present to be read
	if(ret > 0 && ufds.revents&POLLIN){
		int bytes = 0;
		ret = ioctl(_sock, FIONREAD, &bytes);
		if ( ret < 0){
				return 0;
		}
		if ( ret == 0 && bytes != 0){
			return bytes;
		}
	}
	return 0;
#elif defined (DMP_DOS_DJGPP)
	return RxSize;
#endif
}

/* Release any resources being used by this EthernetUDP instance */
void EthernetUDP::stop()
{
#if defined (DMP_LINUX)
	if (_sock == -1)
		return;

	close(_sock);
	_sock = -1;

	EthernetClass::_server_port[0] = 0;
#elif defined (DMP_DOS_DJGPP)
	if (sws == NULL) return;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return;
	
	flush();
	SWS_shutdown(sws->_sock, SWS_SD_BOTH);
	SWS_close(sws->_sock);

	TxSize = 0;
	sws->_sock = SWS_INVALID_SOCKET;
#endif
}

int EthernetUDP::beginPacket(const char *host, uint16_t port)
{
#if defined (DMP_LINUX)
	int ret = 0;
	extern int errno;
	struct hostent *hp;

	if (host == NULL || _sock == -1)
		return -EINVAL;

	hp = gethostbyname(host);
	_offset = 0;
	if (hp == NULL){
			return -ENODEV;
	}
	memcpy(&_sin.sin_addr, hp->h_addr, sizeof(_sin.sin_addr));
	_sin.sin_port = htons(port); //PL : probably useful to have this...
	return 0;
#elif defined (DMP_DOS_DJGPP)
	// Look up the host first
	int ret = 0;
	DNSClient dns;
	IPAddress remote_addr;

	dns.begin(Ethernet.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if (ret == 1)
		return beginPacket(remote_addr, port);
	else
		return ret;
#endif
}

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
#if defined (DMP_LINUX)
	_offset = 0 ;

	_sin.sin_addr.s_addr = ip._sin.sin_addr.s_addr;
	_sin.sin_family = AF_INET;
  	_sin.sin_port = htons(port);

  	return 0;
#elif defined (DMP_DOS_DJGPP)
	uint8_t *ipchar;
	SWS_u_long iplong;
	
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	ipchar = rawIPAddress(ip);
	memcpy(&iplong, ipchar, 4);
	
	bzero(&sws->txaddr, sizeof(sws->txaddr));
    sws->txaddr.sin_family = SWS_AF_INET;
    sws->txaddr.sin_addr.SWS_s_addr = iplong;
    sws->txaddr.sin_port = SWS_htons(port);
	
	TxSize = 0;
	
	return 1;
#endif
}

int EthernetUDP::endPacket()
{
#if defined (DMP_LINUX)
	if ( _sock == -1 )
		return -1;
	return sendUDP();
#elif defined (DMP_DOS_DJGPP)
	int rc, curByte;
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	curByte = 0;
	while (TxSize > 0) {
		rc = SWS_sendto(sws->_sock, &TxBuffer[curByte], TxSize, 0, (struct SWS_sockaddr *)&sws->txaddr, sizeof(sws->txaddr));
		
		if (rc > 0) {
			curByte += rc;
			TxSize -= rc;
		}
		else {
			stop();
			return 0;
		}
	}
	return 1;
#endif
}

#if defined (DMP_LINUX)
int EthernetUDP::sendUDP()
{
	return sendto(_sock, _buffer, _offset, 0, (struct sockaddr*)&_sin, sizeof(_sin));
}

int EthernetUDP::bufferData(const uint8_t *buffer, size_t size)
{
	int written_bytes =  0;
	if  (UDP_TX_PACKET_MAX_SIZE - _offset < size) {
		written_bytes =  UDP_TX_PACKET_MAX_SIZE - _offset;
	} else {
		written_bytes = size;
	}
	memcpy(_buffer + _offset, buffer, written_bytes);
	_offset += written_bytes;
}
#endif

size_t EthernetUDP::write(uint8_t byte)
{
	return write(&byte, 1);
}

size_t EthernetUDP::write(const uint8_t *buffer, size_t size)
{
#if defined (DMP_LINUX)
	uint16_t bytes_written = bufferData(buffer, size);
	return bytes_written;
#elif defined (DMP_DOS_DJGPP)
	size_t bytes_written = 0;
	
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	while (TxSize < UDP_TX_PACKET_MAX_SIZE && size > 0) {
		TxBuffer[TxSize++] = buffer[bytes_written++];
		size--;
	}
	
	return bytes_written;
#endif
}

int EthernetUDP::parsePacket()
{
#if defined (DMP_LINUX)
	// discard any remaining bytes in the last packet
	flush();
	_remaining = 0;
	_remaining = available();
	return _remaining;
#elif defined (DMP_DOS_DJGPP)
	int i, rc, len;
	uint8_t buf[UDP_RX_PACKET_MAX_SIZE];
	
	if (RxSize > 0)
		return RxSize;
	
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	len = sizeof(sws->rxaddr);
	rc = SWS_recvfrom(sws->_sock, buf, UDP_RX_PACKET_MAX_SIZE, 0, (struct SWS_sockaddr *)&sws->rxaddr, &len);
	
	if (rc > 0) {
	
		uint8_t tmpBuf[4];
		
		memcpy(tmpBuf, &sws->rxaddr.sin_addr.SWS_s_addr, 4);
		_remoteIP = tmpBuf;
		_remotePort = SWS_ntohs(sws->rxaddr.sin_port);
		
		i = 0;
		while (rc > 0) {
			RxBuffer[RxTail++] = buf[i++];
			RxSize++;
			if (RxTail >= UDP_RX_PACKET_MAX_SIZE)
				RxTail = 0;
				
			rc--;
		}
	}
	
	return RxSize;
#endif
}

int EthernetUDP::read()
{
#if defined (DMP_LINUX)
	uint8_t b;
	if ( recv(_sock, &b, 1,0) > 0 ) {
		return b;
	} else {
		// No data available
		return -1;
	}
#elif defined (DMP_DOS_DJGPP)
	uint8_t val;
	
	if (read(&val, 1) > 0)
		return val;
		
	return -1;
#endif
}

int EthernetUDP::read(unsigned char* buffer, size_t size)
{
#if defined (DMP_LINUX)
	if (_remaining > 0) {
		int got;

		if (_remaining <= size) {
			// data should fit in the buffer
			got = recvfrom(_sock, buffer, _remaining, 0, NULL, NULL);
		} else {
			// too much data for the buffer,
			// grab as much as will fit
			got = recvfrom(_sock, buffer, size, 0, NULL, NULL);
		}

		if (got > 0) {
			_remaining -= got;
			return got;
		}
	}
	// If we get here, there's no data available or recv failed
	return -1;
#elif defined (DMP_DOS_DJGPP)
	int rc, cur, len;
	uint8_t val;
		
	cur = 0;
	
	if (RxSize > 0) {
	
		while (RxSize > 0 && size > 0) {
			buffer[cur++] = RxBuffer[RxHead++];
			RxSize--;
			size--;
			if (RxHead >= UDP_RX_PACKET_MAX_SIZE)
				RxHead = 0;
		}
	} else if (sws == NULL) {
		return -1;
	} else if (sws->_sock != SWS_INVALID_SOCKET && size > 0) {
	
		len = sizeof(sws->rxaddr);
		rc = SWS_recvfrom(sws->_sock, &buffer[cur], size, 0, (struct SWS_sockaddr *)&sws->rxaddr, &len);
		
		if (rc > 0) {
		
			uint8_t tmpBuf[4];
			
			memcpy(tmpBuf, &sws->rxaddr.sin_addr.SWS_s_addr, 4);
			_remoteIP = tmpBuf;
			_remotePort = SWS_ntohs(sws->rxaddr.sin_port);
			
			size -= rc;
			cur += rc;
		}
		else
			return -1;
	}
	else
		return -1;
	
	return cur;
#endif
}

int EthernetUDP::peek()
{
#if defined (DMP_LINUX)
	return -1;
#elif defined (DMP_DOS_DJGPP)
	if (sws == NULL) return -1;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return -1;
		
	if (!available())
		return -1;
		
	return RxBuffer[RxHead];
#endif
}

void EthernetUDP::flush()
{
#if defined (DMP_LINUX)
	// could this fail (loop endlessly) if _remaining > 0 and recv in read fails?
	// should only occur if recv fails after telling us the data is there, lets
	// hope the w5100 always behaves :)

	while (_remaining) {
		read();
	}
#elif defined (DMP_DOS_DJGPP)
	int rc, len;
	uint8_t val;
	
	RxSize = 0;
	RxHead = 0;
	RxTail = 0;
	
	if (sws == NULL) return;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return;
	
	len = sizeof(sws->rxaddr);
	while ((rc = SWS_recvfrom(sws->_sock, &val, 1, 0, (struct SWS_sockaddr *)&sws->rxaddr, &len)) > 0);
#endif
}

