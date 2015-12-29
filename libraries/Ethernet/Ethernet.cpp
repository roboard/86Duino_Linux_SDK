/*
Ethernet.cpp  Ethernet implementation for x86
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

Modified by CJ Wu on 29 Dec 2015
*/

#include "Ethernet.h"
#include "Dhcp.h"
#if defined (DMP_DOS_DJGPP)
#include "io.h"
#endif

#define MY_TRACE_PREFIX "EthernetClass"

/* Allow the user the opportunity to over-ride this if necessary */
#ifndef ARDUINO_ETH
#define ARDUINO_ETH "eth0"
#endif

// XXX: don't make assumptions about the value of MAX_SOCK_NUM.
#if defined (DMP_LINUX)
uint8_t EthernetClass::_state[MAX_SOCK_NUM] = {
  0, 0, 0, 0 };
uint16_t EthernetClass::_server_port[MAX_SOCK_NUM] = {
  0, 0, 0, 0 };

EthernetClass::EthernetClass()
{
}
#elif defined (DMP_DOS_DJGPP)
uint8_t EthernetClass::MAC_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t *EthernetClass::localMAC()
{
	int i;
	void *mac_io;
	
	if (io_Init() == false)
		;
	else if ((mac_io = io_Alloc(IO_USE_MMIO, 0xFFFFFFB0UL, 0x06UL)) == NULL)
		io_Close();
	else {
		for (i = 0; i < 6; i++) MAC_address[i] = io_In8(mac_io, i);
		io_Free(mac_io);
		io_Close();
	}
	return &MAC_address[0];
}

int EthernetClass::begin(void)
{
	static DhcpClass s_dhcp;
	int ret;

	_dhcp = &s_dhcp;
	
	// Now try to get our config info from a DHCP server
	ret = _dhcp->beginWithDHCP();
	if(ret == 1)
	{
		// We've successfully found a DHCP server and got our configuration info, so set things
		// accordingly
		SwsSock.setIPAddress(_dhcp->getLocalIp().raw_address());
		SwsSock.setGatewayIp(_dhcp->getGatewayIp().raw_address());
		SwsSock.setSubnetMask(_dhcp->getSubnetMask().raw_address());
		SwsSock.setDnsServerIp(_dhcp->getDnsServerIp().raw_address());
	}

	return ret;
}
#endif

int EthernetClass::begin(uint8_t *mac_address)
{
#if defined (DMP_LINUX)
	// In this release - we require the DHCP server to have run and successfully assigned an IP - and we ignore the MAC parameter since this parameter
	// is loaded by the Linux driver by default
	return 1;
#elif defined (DMP_DOS_DJGPP)
	return begin();
#endif
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
#if defined (DMP_LINUX)
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns_server = local_ip;
	//dns_server[3] = 1;
	begin(mac_address, local_ip, dns_server);
#elif defined (DMP_DOS_DJGPP)
	IPAddress dns_server = local_ip;
	dns_server[3] = 1;
	begin(mac_address, local_ip, dns_server);
#endif
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
#if defined (DMP_LINUX)
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = local_ip;
	//gateway[3] = 1;
	begin(mac_address, local_ip, dns_server, gateway);
#elif defined (DMP_DOS_DJGPP)
	IPAddress gateway = local_ip;
	gateway[3] = 1;
	begin(mac_address, local_ip, dns_server, gateway);
#endif
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac_address, local_ip, dns_server, gateway, subnet);
}

void EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
#if defined (DMP_LINUX)
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd;
	int ret;

	_local_ip =  local_ip;
	_dns_server = dns_server;
	_gateway = gateway;
	_subnet = subnet;

        /* Create a channel to the NET kernel. */
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0){
	    return;
	}

        /* get interface flags based on name */
        strncpy(ifr.ifr_name, ARDUINO_ETH, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (ret < 0){
	    close(fd);
	    return;
	}

#if 0
        /* Set broadcast */
	memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = inet_addr("255.255.255.255");
	memcpy(&ifr.ifru_broadaddr, &sin, sizeof(struct sockaddr));
#endif

	/* Bring up i/f */
	if (ifr.ifr_flags  | ~(IFF_UP)){
		ifr.ifr_flags  |= IFF_UP | IFF_RUNNING;
		ret = ioctl(fd, SIOCSIFFLAGS, &ifr);
		if (ret < 0){
		    close(fd);
		    return;
		}

	}

	/* Set local IP */
        memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = _local_ip._sin.sin_addr.s_addr;
        memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));


	/* Set address */
        ioctl(fd, SIOCSIFADDR, &ifr);
        if (ret < 0){
	    close(fd);
	    return;
	}

        /* Set netmask */
        memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = _subnet._sin.sin_addr.s_addr;
        memcpy(&ifr.ifr_netmask, &sin, sizeof(struct sockaddr));

        /* Set address */
	ioctl(fd, SIOCSIFADDR, &ifr);
	if (ret < 0){
		close(fd);
		return;
	}

        close(fd);
#elif defined (DMP_DOS_DJGPP)
	SwsSock.setIPAddress(local_ip._address);
	SwsSock.setDnsServerIp(dns_server._address);
	SwsSock.setGatewayIp(gateway._address);
	SwsSock.setSubnetMask(subnet._address);
	SwsSock.init();
#endif
}

int EthernetClass::maintain()
{
#if 0
	// We expect the DHCP client on Galileo to perform this function out of the box
	// Not re-implementing the code on that basis.
	int rc = DHCP_CHECK_NONE;
	if(_dhcp != NULL){
		//we have a pointer to dhcp, use it
		rc = _dhcp->checkLease();
		switch ( rc ){
			case DHCP_CHECK_NONE:
				//nothing done
				break;
			case DHCP_CHECK_RENEW_OK:
			case DHCP_CHECK_REBIND_OK:
				//we might have got a new IP.
				W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
				W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
				W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
				_dnsServerAddress = _dhcp->getDnsServerIp();
				break;
			default:
				//this is actually a error, it will retry though
			break;
		}
	}
	return rc;
#else
	return 0;
#endif
}

IPAddress EthernetClass::localIP()
{
#if defined (DMP_LINUX)
	IPAddress ret;
	struct ifaddrs *ifaddr = NULL, *ifa = NULL;
	int family, s;
	char ip_addr[NI_MAXHOST];

	// code from the manpage on getifaddrs
	if (getifaddrs(&ifaddr) == -1) {
		return ret;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;

			/* Display interface name and family (including symbolic
				form of the latter for the common families) */

			if (family == AF_INET && !strcmp(ifa->ifa_name, ARDUINO_ETH)){
				s = getnameinfo(ifa->ifa_addr,
								sizeof(struct sockaddr_in),
								ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
						//exit(EXIT_FAILURE);
				}
				ret = (const uint8_t*)ip_addr;
			}
	}
	freeifaddrs(ifaddr);

	return ret;
#elif defined (DMP_DOS_DJGPP)
	IPAddress ret;
	SwsSock.getIPAddress(ret.raw_address());
	return ret;
#endif
}

IPAddress EthernetClass::subnetMask()
{
#if defined (DMP_LINUX)
	IPAddress ret;
	struct ifaddrs *ifaddr = NULL, *ifa = NULL;
	int family, s;
	char ip_addr[NI_MAXHOST];

	// code from the manpage on getifaddrs
	if (getifaddrs(&ifaddr) == -1) {
		return ret;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;

			/* Display interface name and family (including symbolic
				form of the latter for the common families) */

			if (family == AF_INET && !strcmp(ifa->ifa_name, ARDUINO_ETH)){
				s = getnameinfo(ifa->ifa_netmask,
								sizeof(struct sockaddr_in),
								ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
						//exit(EXIT_FAILURE);
				}
				ret = (const uint8_t*)ip_addr;
			}
	}
	freeifaddrs(ifaddr);
	return ret;
#elif defined (DMP_DOS_DJGPP)
	IPAddress ret;
	SwsSock.getSubnetMask(ret.raw_address());
	return ret;
#endif
}

IPAddress EthernetClass::gatewayIP()
{
#if defined (DMP_LINUX)
	IPAddress ret;
	return ret;
#elif defined (DMP_DOS_DJGPP)
	IPAddress ret;
	SwsSock.getGatewayIp(ret.raw_address());
	return ret;
#endif
}

IPAddress EthernetClass::dnsServerIP()
{
#if defined (DMP_LINUX)
	return _dnsServerAddress;
#elif defined (DMP_DOS_DJGPP)
	IPAddress ret;
	SwsSock.getDnsServerIp(ret.raw_address());
	return ret;
#endif
}

EthernetClass Ethernet;
