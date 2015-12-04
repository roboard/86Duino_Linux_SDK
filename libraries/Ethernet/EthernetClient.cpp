/*
EthernetClient.cpp  Ethernet client implementation for x86
Copyright (C) 2014 Intel Corporation

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */

// Arduino includes
extern "C" {
	#include "string.h"
}
#include "Arduino.h"
#include "Ethernet.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dns.h"
 
#if defined (DMP_LINUX)
// Standard system includes
#include <errno.h>			// -EINVAL, -ENODEV
#include <netdb.h>			// gethostbyname
#include <sys/ioctl.h>		// ioctl
#include <sys/poll.h>		// poll
#include <sys/types.h>		// connect
#include <sys/socket.h>		// connect

// Firmware includes specific to galileo

#define MY_TRACE_PREFIX "EthernetClient"
#endif

#if defined (DMP_LINUX)
uint16_t EthernetClient::_srcport = 1024;
#elif defined (DMP_DOS_DJGPP)
static int swssock_connect(SWS_SOCKET s, const SWS_sockaddr *addr, int size)
{
	int rc;
	SWS_fd_set fds;
	struct SWS_timeval seltime;
	
	SWS_connect(s, addr, size);

	SWS_FdZero(&fds);
	SWS_FdSet(s,&fds);

	seltime.tv_sec = 10;
	seltime.tv_usec = 0;

	if (SWS_select(NULL, &fds, NULL, &seltime) <= 0)
		return -1;

	return 0;
}
#endif

EthernetClient::EthernetClient()
{
#if defined (DMP_LINUX)
    // full client mode - not connected
	_sock = -1;
	connect_true = false;
	_pCloseServer = NULL;
#elif defined (DMP_DOS_DJGPP)
	_id = -1;
	pServer = NULL;
	sws = (struct SwsSockInfo*)malloc(sizeof(struct SwsSockInfo));
	if (sws) {
		memset(sws, 0, sizeof(struct SwsSockInfo));
		sws->_sock = SWS_INVALID_SOCKET;
	}
	_RegLen = 0;
#endif
}

#if defined (DMP_LINUX)
EthernetClient::EthernetClient(uint8_t sock)
{
	// being explicitely initialised by external logic to _sock = sock
	_sock = sock;
	connect_true = true;
	_pCloseServer = NULL;
}

static int _connect(int _sock, struct sockaddr_in * psin, unsigned int len)
{
	extern int errno;
	int ret;


	ret = connect(_sock, (struct sockaddr*)psin, len);
	if (ret < 0){
		close(_sock);
		return ret;
	}
	return ret;
}
#elif defined (DMP_DOS_DJGPP)
EthernetClient::EthernetClient(struct SwsSockInfo *info)
{
	_id = -1;
	pServer = NULL;
	sws = info;
	_RegLen = 0;
}

int EthernetClient::writeNB(uint8_t b)
{
	return writeNB(&b, 1);
}

int EthernetClient::writeNB(const uint8_t *buf, size_t size)
{
	int ret = -1;
	
	if (sws == NULL) return ret;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return ret;
	
	ret = SWS_send(sws->_sock, buf, size, 0);
	if (ret < 0 && SWS_GetLastError() == SWS_EWOULDBLOCK)
		ret = 0;
		
	return ret;
}
#endif

int EthernetClient::connect(const char* host, uint16_t port)
{
#if defined (DMP_LINUX)
	// Look up the host first
	struct hostent *hp;
	int ret = 0;
	extern int errno;

	if (host == NULL || _sock != -1)
		return 0;

	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock < 0){
		return 0;
	}

	hp = gethostbyname(host);
	if (hp == NULL){
			return 0;
	}
	memcpy(&_sin.sin_addr, hp->h_addr, sizeof(_sin.sin_addr));
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);

	ret = _connect(_sock, &_sin, sizeof(_sin));
	if (ret < 0){
		return 0;
	}

	if ( ret == 0)
		connect_true = true;

	return 1;
#elif defined (DMP_DOS_DJGPP)
	int ret = 0;
	DNSClient dns;
	IPAddress remote_addr;
	IPAddress dns_server;

	dns.begin(Ethernet.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if (ret == 1)
		return connect(remote_addr, port);
	
	return ret;
#endif
}

int EthernetClient::connect(IPAddress ip, uint16_t port)
{
#if defined (DMP_LINUX)
  // Look up the host first
  	int ret = 0;
  	extern int errno;
  	int on = 1;

  	if (_sock != -1)
  		return 0;

  	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  	if (_sock < 0){
  		return 0;
  	}

	_sin.sin_addr.s_addr = ip._sin.sin_addr.s_addr;
  	_sin.sin_family = AF_INET;
  	_sin.sin_port = htons(port);

  	ret = _connect(_sock, &_sin, sizeof(_sin));
  	if (ret < 0){
		return 0;
	}

	// Set non-blocking - required for poll() on this socket to work and for child sockets
	ret = ioctl(_sock, FIONBIO, (char *)&on);
	if (ret < 0) {
		close(_sock);
		return 0;
	}
	if ( ret == 0)
		connect_true = true;

	return 1;
#elif defined (DMP_DOS_DJGPP)
	struct SWS_sockaddr_in pin;
	int yes = 1, no = 0;
	uint8_t *ipchar;
	SWS_u_long iplong, lArg = 1;
	
	if (sws == NULL) return 0;
	
	if (sws->_sock != SWS_INVALID_SOCKET)
		stop();
	
	ipchar = rawIPAddress(ip);
	memcpy(&iplong, ipchar, 4);
	
	bzero(&pin, sizeof(pin));
    pin.sin_family = SWS_AF_INET;
    pin.sin_addr.SWS_s_addr = iplong;
    pin.sin_port = SWS_htons(port);
	
	sws->_sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0);
    if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
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
	
	if (SWS_ioctl(sws->_sock, SWS_FIONBIO, &lArg) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}

    if (swssock_connect(sws->_sock, (const SWS_sockaddr*)&pin, sizeof(pin)) == -1) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
    }
	
	_id = -1;
	pServer = NULL;
	
	return 1;
#endif
}

size_t EthernetClient::write(uint8_t b) {
  return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *buf, size_t size)
{
#if defined (DMP_LINUX)
	extern int errno;

 	if (_sock < 0) {
		return 0;
	}

	if (send(_sock, buf, size, 0) < 0) {
		_sock = -1;
    		return 0;
	}

	return size;
#elif defined (DMP_DOS_DJGPP)
	int ret;
	size_t n = 0, res = size;
	
	while (n < size) {
		if ((ret = writeNB(&buf[n], res)) < 0)
			break;
		n += ret;
		res -= ret;
	}
	
	return n;
#endif
}

int EthernetClient::available()
{
#if defined (DMP_LINUX)
	struct pollfd ufds;
	int ret = 0;
	extern int errno;
	int    timeout = 5000;	// milliseconds

	if (_sock == -1){
		return 0;
	}

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
		    	if (_inactive_counter)
		    		*_inactive_counter = 0;
			return 1;
		}else{
			if (_inactive_counter == NULL)
				return 0;

			// Increment inactivity counter
			(*_inactive_counter)++;

			// counter exceeded nuke connection
			if ( *_inactive_counter >= CLIENT_MAX_INACTIVITY_RETRIES){
				stop();
			}
		}
	}

	return 0;
#elif defined (DMP_DOS_DJGPP)
	int rc, cur = 0;
	uint8_t buf[BUFFER_SIZE];
	
	if (_RegLen > 0)
		cur = 1;
	
	if (sws == NULL) return 0;
	if (sws->_sock != SWS_INVALID_SOCKET) {
	
		rc = SWS_recv(sws->_sock, buf, BUFFER_SIZE-cur, SWS_MSG_PEEK);
		
		if (rc > 0)
			cur += rc;
	}
	
	return cur;
#endif
}

int EthernetClient::read()
{
#if defined (DMP_LINUX)
	uint8_t b;
	if ( recv(_sock, &b, 1, 0) > 0 ) {
		return b;
	} else {
		// No data available
		return -1;
	}
#elif defined (DMP_DOS_DJGPP)
	int rc;
	uint8_t val;
	
	if (read(&val, 1) == 0)
		return -1;
	
	return val;
#endif
}

int EthernetClient::read(uint8_t *buf, size_t size)
{
#if defined (DMP_LINUX)
	return recv(_sock, buf, size, 0);
#elif defined (DMP_DOS_DJGPP)
	int rc, cur = 0;
	
	if (_RegLen > 0) {
		buf[cur++] = _RegBuf;
		_RegLen = 0;
		size--;
	}
	
	if (sws == NULL) return 0;
	if (sws->_sock != SWS_INVALID_SOCKET && size > 0) {
		
		rc = SWS_recv(sws->_sock, &buf[cur], size, 0);
		if (rc > 0)
			cur += rc;
	}
	
	return cur;
#endif
}

int EthernetClient::peek()
{
#if defined (DMP_LINUX)
	return -1;
#elif defined (DMP_DOS_DJGPP)
	int rc;
	uint8_t val;
	
	if (_RegLen > 0) {
		val = _RegBuf;
		_RegLen = 0;
		return val;
	}
	
	if (sws == NULL) return -1;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return -1;
	
	if (SWS_recv(sws->_sock, &val, 1, SWS_MSG_PEEK) <= 0)
		return -1;
	
	return val;
#endif
}

void EthernetClient::flush()
{
#if defined (DMP_LINUX)
	while (available())
		read();
#elif defined (DMP_DOS_DJGPP)
	uint8_t val;
	
	if (sws == NULL) return;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return;
		
	while (SWS_recv(sws->_sock, &val, 1, 0) > 0);
#endif
}

void EthernetClient::stop()
{
#if defined (DMP_LINUX)
	if (_sock < 0)
		return;

	connect_true = false;
	if (_inactive_counter != NULL)
		*_inactive_counter = 0;
	if(_sock != -1){
		close(_sock);
		_sock = -1;

		// Sketches do a Ethernet client = server.available() - which means a copy constructor is used
		// Hence any server which populated a pclients[idx] - won't actually see the socket closed - since
		// his client was copied - not given a pointer to.... pcleint[] in EthernetServer.cpp _sock != -1 at this point
		// make it so !
		if(_pCloseServer != NULL){
		    	_pCloseServer->closeNotify(this->id);
		}
	}
#elif defined (DMP_DOS_DJGPP)
	if (sws) {
		if (sws->_sock != SWS_INVALID_SOCKET) {
			SWS_shutdown(sws->_sock, SWS_SD_BOTH);
			SWS_close(sws->_sock);
			sws->_sock = SWS_INVALID_SOCKET;
		}
	}
	if (pServer != NULL) {
		pServer->closeServerSocket(this->_id);
	}
#endif
}

uint8_t EthernetClient::connected()
{
#if defined (DMP_LINUX)
	return connect_true == true;
#elif defined (DMP_DOS_DJGPP)
	int rc;
	uint8_t val;
	
	if (available() > 0)
		return 1;
	
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	rc = SWS_recv(sws->_sock, &val, 1, 0);
	if (rc == 0)
		return 0;
	
	if (rc > 0) {
		_RegBuf = val;
		_RegLen = 1;
	}
	
	return 1;
#endif
}

uint8_t EthernetClient::status()
{
#if defined (DMP_LINUX)
	return _sock == -1;
#elif defined (DMP_DOS_DJGPP)
	if (sws == NULL) return sws != NULL;
	return sws->_sock != SWS_INVALID_SOCKET;
#endif
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool() {
#if defined (DMP_LINUX)
	return _sock != -1;
#elif defined (DMP_DOS_DJGPP)
	if (sws == NULL) return sws != NULL;
		return sws->_sock != SWS_INVALID_SOCKET;
#endif
}
