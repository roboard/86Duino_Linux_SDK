#ifndef ethernetserver_h
#define ethernetserver_h

#include "Server.h"
#include "Ethernet.h"
#if defined (DMP_LINUX)
#include "EthernetClient.h"
#endif

class EthernetClient;

class EthernetServer :
public Server {
private:
#if defined (DMP_LINUX)
	uint16_t _port;
	void accept();
	int _sock;
	int _scansock;
	struct sockaddr_in _sin;
	struct sockaddr_in _cli_sin;
	bool _init_ok;
	EthernetClient *pclients;
	int *_pcli_inactivity_counter;
#elif defined (DMP_DOS_DJGPP)
	struct SwsSockInfo *sws;
	struct SwsSockInfo *make_new_client(struct SwsSockInfo *info);
	uint16_t _port;
	EthernetClient Clients[MAX_SOCK_NUM];
#endif
public:
	EthernetServer(uint16_t);
	~EthernetServer();
	EthernetClient available();
	virtual void begin();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	using Print::write;
#if defined (DMP_LINUX)
	void closeNotify(int idx);
#elif defined (DMP_DOS_DJGPP)
	void closeServerSocket(int idx);
#endif
};

#endif
