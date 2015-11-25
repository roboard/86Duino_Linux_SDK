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

// Standard system includes
#include <errno.h>			// -EINVAL, -ENODEV
#include <netdb.h>			// gethostbyname
#include <sys/ioctl.h>		// ioctl
#include <sys/poll.h>		// poll
#include <sys/types.h>		// connect
#include <sys/socket.h>		// connect

// Firmware includes specific to galileo

#define MY_TRACE_PREFIX "EthernetClient"

// Arduino includes
extern "C" {
	#include "string.h"
}
#include "Arduino.h"
#include "Ethernet.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dns.h"

uint16_t EthernetClient::_srcport = 1024;

EthernetClient::EthernetClient() : _sock(-1)
{
    	// full client mode - not connected
	connect_true = false;
	_pCloseServer = NULL;
}

EthernetClient::EthernetClient(uint8_t sock) : _sock(sock)
{
	// being explicitely initialised by external logic to _sock = sock
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

int EthernetClient::connect(const char* host, uint16_t port)
{
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
}

int EthernetClient::connect(IPAddress ip, uint16_t port)
{
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
}

size_t EthernetClient::write(uint8_t b) {
  return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *buf, size_t size)
{
	extern int errno;

 	if (_sock < 0) {
		return 0;
	}

	if (send(_sock, buf, size, 0) < 0) {
		_sock = -1;
    		return 0;
	}

	return size;
}

int EthernetClient::available()
{
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
}

int EthernetClient::read()
{
	uint8_t b;
	if ( recv(_sock, &b, 1, 0) > 0 ) {
		return b;
	} else {
		// No data available
		return -1;
	}
}

int EthernetClient::read(uint8_t *buf, size_t size)
{
	return recv(_sock, buf, size, 0);
}

int EthernetClient::peek()
{
	# if 0
		uint8_t b;
		// Unlike recv, peek doesn't check to see if there's any data available, so we must

		if (!available())
			return -1;
		::peek(_sock, &b);

		return b;
	#else
		// Not implemented - the only way to do this is to have a thread and some buffers like in Serial
		// If someone really wants that functionality - at liberty to go and implement
		// natively read/write don't peek and I'm not aware of an ioctl() that would let you do that on a socket
		return -1;
	#endif
}

void EthernetClient::flush()
{
	while (available())
		read();
}

void EthernetClient::stop()
{
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
}

uint8_t EthernetClient::connected()
{
	return connect_true == true;
}

uint8_t EthernetClient::status()
{
	return _sock == -1;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool() {
  return _sock != -1;
}
