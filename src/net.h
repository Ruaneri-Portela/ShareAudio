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
	unsigned short int asServer;
	unsigned int port;
	char* host;
	short int device;
	size_t dataSize;
	size_t delay;
	netCtx* ctx;
	dataHandshake* dh;
	void* thread;
} connectParam;

extern char data[DATASIZE + 3];

extern char *msg;

unsigned short int SA_NetSetupClient(connectParam* parms);

void* SA_NetInit(int port, const char* host, int asClient, int device, dataHandshake* dh);

void SA_NetClose(void* thread);
