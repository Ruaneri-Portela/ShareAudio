typedef struct netCtx
{
	SOCKET clientSocket;
	SOCKET srvSocket;
	SOCKADDR_IN srvAddr;
} netCtx;

typedef struct connectParam
{
	saConnection* conn;

	EVP_CIPHER_CTX* encryptCtx;
	EVP_CIPHER_CTX* decryptCtx;

	size_t dataSize;
	size_t delay;
	netCtx* ctx;
} connectParam;

unsigned short int SA_NetSetupClient(connectParam* parms);

void SA_NetInit(saConnection* conn);

void SA_NetClose(void* thread, saConnection* conn);