#include <windows.h>

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

extern HANDLE closeThread;

unsigned short int SA_NetSetupClient(connectParam* parms);

HANDLE SA_NetInit(int port, char* host, size_t asClient, int device);

void SA_NetClose(void* hThread);
