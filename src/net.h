#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#else
#include <netinet/in.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define ADDRESS_FAMILY int

#endif
typedef struct netCtx
{
	SOCKET clientSocket;
	SOCKET srvSocket;
	SOCKADDR_IN srvAddr;
} netCtx;

typedef struct connectParam
{
	int asServer;
	int port;
	char *host;
	int dataSize;
	size_t delay;
	netCtx *ctx;
} connectParam;

extern size_t sessionPacket;

extern size_t totalPacketSrv;

extern void* closeThread;

unsigned short int SA_NetSetupClient(connectParam* parms);

void* SA_NetInit(unsigned int port, const char* host, unsigned short int asClient, short int device);

void SA_NetClose(void* thread);
