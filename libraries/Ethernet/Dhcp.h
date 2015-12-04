// DHCP Library v0.3 - April 25, 2009
// Author: Jordan Terrell - blog.jordanterrell.com
//
// Modified by Denny Yang on 25 Sep 2013:
//    - making to compile in x86 for Intel Galileo Arduino IDE

#ifndef Dhcp_h
#define Dhcp_h

#include "utility/SwsSock.h"

#if defined (DMP_DOS_DJGPP)
#include "EthernetUdp.h"

class DhcpClass {

public:
	IPAddress getLocalIp();
	IPAddress getSubnetMask();
	IPAddress getGatewayIp();
	IPAddress getDnsServerIp();

	int beginWithDHCP(unsigned long timeout = 10000, unsigned long responseTimeout = 4000);
	int checkLease();
};
#endif

#endif
