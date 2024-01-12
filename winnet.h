#include <windows.h>

typedef struct srvCtx
{
	SOCKET clientSocket;
	SOCKET srvSocket;
	SOCKADDR_IN srvAddr;
} srvCtx;

typedef struct connectParam
{
	int asServer;
	int port;
	char* host;
	int dataSize;
	size_t delay;
	srvCtx* ctx;
} connectParam;

extern HANDLE closeThread;

HANDLE initNet(int port, char* host, size_t asClient);

void closeNet(void *hThread);
