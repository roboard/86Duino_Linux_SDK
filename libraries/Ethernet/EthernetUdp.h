/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.c/socket.h
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/
 *
 * NOTE: UDP is fast, but has some important limitations (thanks to Warren Gray for mentioning these)
 * 1) UDP does not guarantee the order in which assembled UDP packets are received. This
 * might not happen often in practice, but in larger network topologies, a UDP
 * packet can be received out of sequence.
 * 2) UDP does not guard against lost packets - so packets *can* disappear without the sender being
 * aware of it. Again, this may not be a concern in practice on small local networks.
 * For more information, see http://www.cafeaulait.org/course/week12/35.html
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
 * Modified by CJ Wu on 29 Dec 2015
 */

#ifndef ethernetudp_h
#define ethernetudp_h

#include "utility/SwsSock.h"
#include <Udp.h>
#if defined (DMP_LINUX)
#include <netinet/in.h>
#define UDP_TX_PACKET_MAX_SIZE 1024
#elif defined (DMP_DOS_DJGPP)
#define UDP_TX_PACKET_MAX_SIZE (512)
#define UDP_RX_PACKET_MAX_SIZE (512)
#endif

class EthernetUDP : public UDP {
private:
//	uint8_t _sock;  // socket ID for Wiz5100
	uint16_t _port; // local port to listen on
	IPAddress _remoteIP; // remote IP address for the incoming packet whilst it's being processed
	uint16_t _remotePort; // remote port for the incoming packet whilst it's being processed
#if defined (DMP_LINUX)
	uint16_t _offset; // offset into the packet being sent
	uint16_t _remaining; // remaining bytes of incoming packet yet to be processed
	struct sockaddr_in _sin;
#elif defined (DMP_DOS_DJGPP)
	struct SwsSockInfo *sws;
	uint8_t TxBuffer[UDP_TX_PACKET_MAX_SIZE];
	uint8_t RxBuffer[UDP_RX_PACKET_MAX_SIZE];
	int TxSize;
	int RxSize;
	int RxHead;
	int RxTail;
#endif
public:
	EthernetUDP();  // Constructor
	virtual uint8_t begin(uint16_t);	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
	virtual void stop();  // Finish with the UDP socket
	virtual int beginPacket(IPAddress ip, uint16_t port);
	virtual int beginPacket(const char *host, uint16_t port);
	virtual int endPacket();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);
	using Print::write;
	virtual int parsePacket();
	virtual int available();
	virtual int read();
	virtual int read(unsigned char* buffer, size_t len);
	virtual int read(char* buffer, size_t len) { return read((unsigned char*)buffer, len); };
	virtual int peek();
	virtual void flush();
	virtual IPAddress remoteIP() { return _remoteIP; };
	virtual uint16_t remotePort() { return _remotePort; };
#if defined (DMP_LINUX)
	virtual int listen();
	int _sock; 
	uint8_t _buffer[UDP_TX_PACKET_MAX_SIZE];
	virtual int sendUDP();
	virtual int bufferData(const uint8_t *buffer, size_t size);
#elif defined (DMP_DOS_DJGPP)
	~EthernetUDP();
#endif
};

#endif
