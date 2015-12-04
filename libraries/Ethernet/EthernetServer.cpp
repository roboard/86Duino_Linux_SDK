/*
EthernetServer.cpp  Ethernet Arduino Server implementation for x86
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
extern "C" {
#include "string.h"
}

#include "EthernetClient.h"
#include "EthernetServer.h"

#if defined (DMP_LINUX)
#include "Ethernet.h"
// Standard system includes
#include <assert.h>
#include <errno.h>			// -EINVAL, -ENODEV
#include <netdb.h>			// gethostbyname
#include <sys/poll.h>
#include <sys/types.h>		// connect
#include <sys/socket.h>		// connect

#define MY_TRACE_PREFIX "EthernetServer"
#endif

EthernetServer::EthernetServer(uint16_t port)
{
#if defined (DMP_LINUX)
	_port = port;
	_sock = -1;
	_init_ok = false;
	pclients = new EthernetClient[MAX_SOCK_NUM];
	_pcli_inactivity_counter = new int[MAX_SOCK_NUM];
	_scansock = 0;

	if (pclients == NULL){
		return;
	}
	for(int sock = 0; sock < MAX_SOCK_NUM; sock++){
		_pcli_inactivity_counter[sock] = 0;
		pclients[sock].id = sock;
	}
#elif defined (DMP_DOS_DJGPP)
	int i;
	_port = port;
	sws = (struct SwsSockInfo*)malloc(sizeof(struct SwsSockInfo));
	if (sws) {
		memset(sws, 0, sizeof(struct SwsSockInfo));
		sws->_sock = SWS_INVALID_SOCKET;
	}
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].sws == NULL) continue;
		Clients[i].sws->_sock = SWS_INVALID_SOCKET;
		Clients[i]._id = -1;
		Clients[i].pServer = this;
	}
#endif
}

EthernetServer::~EthernetServer()
{
#if defined (DMP_LINUX)
    if (pclients != NULL){
    	delete [] pclients;
    	pclients = NULL;
    }

    if(_pcli_inactivity_counter != NULL){
    	delete [] _pcli_inactivity_counter;
    	_pcli_inactivity_counter = NULL;
    }
#elif defined (DMP_DOS_DJGPP)
	for (int i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].sws == NULL) continue;
		free(Clients[i].sws);
		Clients[i].sws = NULL;
	}
	if (sws) {
		free(sws);
		sws = NULL;
	}
#endif
}

void EthernetServer::begin()
{
#if defined (DMP_LINUX)
	int ret;
	extern int errno;

	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock < 0){
		return;
	}

	_sin.sin_addr.s_addr = INADDR_ANY;
	_sin.sin_family = AF_INET;
  	_sin.sin_port = htons(_port);

	ret = bind(_sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if ( ret < 0){
		return;
	}

	//socket(sock, SnMR::TCP, _port, 0);
	ret = listen(_sock, MAX_SOCK_NUM);
	if ( ret < 0){
		return;
	}

	for(int sock = 0; sock < MAX_SOCK_NUM; sock++)
		EthernetClass::_server_port[sock] = _port;

	// mark as available
	_init_ok = true;
#elif defined (DMP_DOS_DJGPP)
	int idx;
	struct SWS_sockaddr_in sin;
	SWS_u_long lArg = 1;
	int yes = 1, no = 0;
	
	if (sws == NULL) return;
	sws->_sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0);
    if (sws->_sock == SWS_INVALID_SOCKET) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		return;
    }
	
	bzero(&sin, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = SwsSock.getULLocalIp();
    sin.sin_port = SWS_htons(_port);
	
	if (SWS_bind(sws->_sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
    }

    if (SWS_listen(sws->_sock, MAX_SOCK_NUM) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
    }
	
	if (SWS_ioctl(sws->_sock, SWS_FIONBIO, &lArg) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&no, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
#endif
}

#if defined (DMP_LINUX)
static int _accept(int sock, struct sockaddr * psin, socklen_t * psize)
{
	return accept(sock, psin, psize);
}

void EthernetServer::accept()
{
	struct pollfd ufds;
	int ret = 0, size_val, success = 0;
	extern int errno;

	if (_sock == -1)
		return;

	ufds.fd = _sock;
	ufds.events = POLLIN;
	ufds.revents = 0;

	ret = poll(&ufds, 1, 0);
	if ( ret < 0 ){
		_sock = -1;
		close(_sock);
		return;
	}

	if(ufds.revents&POLLIN){
		//trace_debug("%s in activity on socket %d - calling accept()", __func__, _sock);
		size_val = sizeof(_cli_sin);
		ret = _accept(_sock, (struct sockaddr*)&_cli_sin, (socklen_t*)&size_val);

		if ( ret < 0){
			close(_sock);
			_sock = -1;
			return;
		}

		for(int sock = 0; sock < MAX_SOCK_NUM && success == 0; sock++){
			if (pclients[sock]._sock == -1){
				pclients[sock]._sock = ret;
				pclients[sock]._pCloseServer = this;
				pclients[sock].connect_true = true;
				pclients[sock]._inactive_counter = &_pcli_inactivity_counter[sock];
				success = 1;
			}
		}
		if ( success == 0 ){
		}
	}
}
#elif defined (DMP_DOS_DJGPP)
struct SwsSockInfo *EthernetServer::make_new_client(struct SwsSockInfo *info)
{
	SWS_SOCKET tempsock;
	int addrsize, idx;
	int yes = 1;
	SWS_u_long lArg = 1;
	struct SWS_sockaddr_in pin;
	
	for (idx = 0; idx < MAX_SOCK_NUM; idx++) {
		if (Clients[idx].sws == NULL) continue;
		if (Clients[idx].sws->_sock == SWS_INVALID_SOCKET)
			break;
	}
	
	addrsize = sizeof(pin);
	if ((tempsock = SWS_accept(sws->_sock, (struct SWS_sockaddr *)&pin, &addrsize)) < 0)
		return NULL;
	
	if (SWS_ioctl(tempsock, SWS_FIONBIO, &lArg) != 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (SWS_setsockopt(tempsock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (SWS_setsockopt(tempsock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (idx >= MAX_SOCK_NUM) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	Clients[idx].sws->_sock = tempsock;
	Clients[idx]._id = idx;
	
	return Clients[idx].sws;
}
#endif

EthernetClient EthernetServer::available()
{
#if defined (DMP_LINUX)
	accept();

	// Scan for next connection - meaning don't return the same one each time
	for(int sock = 0; sock < MAX_SOCK_NUM; sock++){
		if (pclients[sock]._sock != -1 && sock != _scansock){
			//trace_debug("Returning socket entity %d socket %d", sock, pclients[sock]._sock);
			_scansock = sock;
			return pclients[sock];
		}
	}

	// scan for any connection
	for(int sock = 0; sock < MAX_SOCK_NUM; sock++){
		if (pclients[sock]._sock != -1){
			//trace_debug("Returning socket entity %d socket %d", sock, pclients[sock]._sock);
			_scansock = sock;
			return pclients[sock];
		}
	}

	//trace_debug("%s no client to return", __func__);
	_scansock = 0;
	return pclients[0];

#elif defined (DMP_DOS_DJGPP)
	struct SwsSockInfo *temp;
	SWS_SOCKET tempsock;
	SWS_SOCKET socks[MAX_SOCK_NUM+1];
	int i, j;
	struct SWS_timeval seltime;
	
	if (sws == NULL) return EthernetClient(NULL);
	if (sws->_sock != SWS_INVALID_SOCKET)
	{
		socks[0] = sws->_sock;
		SWS_FdZero(&sws->rfds);
		SWS_FdSet(sws->_sock, &sws->rfds);
		for (i = 0; i < MAX_SOCK_NUM; i++) {
			if (Clients[i].sws == NULL) continue;
			socks[i+1] = Clients[i].sws->_sock;
			
			if (Clients[i].sws->_sock != SWS_INVALID_SOCKET){
				SWS_FdSet(Clients[i].sws->_sock, &sws->rfds);
				}
		}

		seltime.tv_sec = 0;
		seltime.tv_usec = 0;
		if (!SWS_select(&sws->rfds, NULL, NULL, &seltime))
			return EthernetClient(NULL);
		
		for (i = 0; i < MAX_SOCK_NUM+1; i++) {
			
			if (SWS_FdIsSet(socks[i], &sws->rfds)) {
				
				if (socks[i] == sws->_sock) {
					temp =  make_new_client(sws);
					tempsock = (temp == NULL) ? (SWS_INVALID_SOCKET) : (temp->_sock);
				} else
					tempsock = socks[i];
				
				if (tempsock != SWS_INVALID_SOCKET && socks[i] != sws->_sock) {
					for (j = 0; j < MAX_SOCK_NUM; j++) {
						if (Clients[j].sws == NULL) continue;
						if (Clients[j].sws->_sock == tempsock) {
							uint8_t val;
							
							if (SWS_recv(Clients[j].sws->_sock, &val, 1, SWS_MSG_PEEK) <= 0)
								Clients[j].stop();
							
							return Clients[j];
						}
					}
				}
			}
		}
	}
	
	return EthernetClient(NULL);
#endif
}

size_t EthernetServer::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size)
{
#if defined (DMP_LINUX)
	size_t n = 0;

	//accept();

	// This routine writes the given data to all current clients
	for(int sock = 0; sock < MAX_SOCK_NUM; sock++){
		if (pclients[sock]._sock != -1){
		    	n += pclients[sock].write(buffer, size);
		}
	}
	return n;
#elif defined (DMP_DOS_DJGPP)
	int i;
	size_t n = 0;
	
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].status())
			n += Clients[i].write(buffer, size);
	}
	
	return n;
#endif
}

#if defined (DMP_LINUX)
void EthernetServer::closeNotify(int idx)
{
    if (idx < MAX_SOCK_NUM)
    	pclients[idx]._sock = -1;
}
#elif defined (DMP_DOS_DJGPP)
void EthernetServer::closeServerSocket(int idx)
{
	if (idx >= 0 && idx < MAX_SOCK_NUM) {
		if (Clients[idx].sws != NULL) {
			Clients[idx].sws->_sock = SWS_INVALID_SOCKET;
		}
	}
}
#endif
