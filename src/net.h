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

extern char* msg;

unsigned short int SA_NetSetupClient(connectParam* parms);

void* SA_NetInit(int port, const char* host, int asClient, int device, dataHandshake* dh);

void SA_NetClose(void* thread);
