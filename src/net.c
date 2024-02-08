#include "config.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(LINUX)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include "threads.h"
#include "audio.h"
#include "log.h"
#include "data.h"

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
	const char* host;
	short int device;
	size_t dataSize;
	size_t delay;
	netCtx* ctx;
	dataHandshake* dh;
	void* thread;
} connectParam;

char data[DATASIZE + 3];

char* msg = NULL;

static const unsigned char confirmConn[2] = { 0xFF, '\0' };

#if defined(WINDOWS)
static WSADATA wsaData;
static void SA_WinNetEnd(void* parms)
{
	free(((connectParam*)parms)->ctx);
	free(parms);
	if (WSACleanup() != 0)
	{
		SA_Log("WSA End failed!", LOG_NET, LOG_CLASS_WARNING);
	}
	else
	{
		SA_Log("WSA End", LOG_NET, LOG_CLASS_INFO);
	}
}

static void SA_WinNetOpen()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		SA_Log("Winsock2 failled!", LOG_NET, LOG_CLASS_INFO);
	}
	else
	{
		SA_Log("WSA Start", LOG_NET, LOG_CLASS_INFO);
	}
}
#else
static void SA_WinNetEnd(void* parms)
{
	free(((connectParam*)parms)->ctx);
	free(parms);
}
static void SA_WinNetOpen()
{
}
#endif

static void SA_NetResolveHost(connectParam* parm, ADDRESS_FAMILY family)
{
	unsigned short int detectIp = SA_DataDetectIsIp(parm->host, parm->asServer);
	switch (detectIp)
	{
	case 0:
		SA_Log("Try Resolve by host", LOG_NET, LOG_CLASS_INFO);
		break;
	case 1:
		SA_Log("IPv4 Detected", LOG_NET, LOG_CLASS_INFO);
		break;
	case 2:
		SA_Log("IP Forbbiden on Client", LOG_NET, LOG_CLASS_ERROR);
		break;
	}
	const char* hostLocal = parm->host;
	struct addrinfo* res = NULL;
	char* ip = NULL;
	if (detectIp == 0)
	{
		hostLocal = NULL;
		int tam = sizeof(char) * 16;
		char* ip = malloc(tam);
		ip == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
		getaddrinfo(parm->host, 0, 0, &res);
		for (struct addrinfo* ptr = res; ptr != NULL; ptr = ptr->ai_next)
		{
			if (ptr->ai_addr->sa_family == AF_INET)
			{
				struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
				if (ip != NULL)
				{
					inet_ntop(AF_INET, &(ipv4->sin_addr), ip, 16);
					hostLocal = ip;
					break;
				}
			}
		}
	}
	parm->ctx->srvAddr.sin_family = family;
	parm->ctx->srvAddr.sin_port = htons(parm->port);
	if (hostLocal != NULL && inet_pton(AF_INET, hostLocal, &parm->ctx->srvAddr.sin_addr) == 0)
	{
		if (res != NULL)
		{
			free(res);
		}
		if (ip != NULL)
		{
			free(ip);
		}
		SA_Log("Invalid host address", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		const char* msgPrefix;
		if (parm->asServer == 1)
		{
			msgPrefix = "Listen on: ";
		}
		else
		{
			msgPrefix = "Connect to host: ";
		}
		char* logMsg = SA_DataConcatString(msgPrefix, parm->host);
		SA_Log(logMsg, LOG_NET, LOG_CLASS_INFO);
		free(logMsg);
	}
}

static void SA_NetServerRecv(connectParam* parms) {
	int failCount = 0;
	char msgLocal[DATASIZE + 3];
	char* msgStream = NULL;
	int rounds = 0;
	while (parms->thread != NULL && parms->ctx->clientSocket != 0) {
		if (recv(parms->ctx->clientSocket, msgLocal, DATASIZE + 2, 0) == SOCKET_ERROR)
		{
			SA_Log("Revc failed!", LOG_NET, LOG_CLASS_DEBUG);
			if (failCount > 10) {
				SA_Log("Revc failed!", LOG_NET, LOG_CLASS_WARNING);
				break;
			}
			failCount++;
		}
		else
		{
			SA_DataRevcProcess(&rounds, &msgStream, msgLocal, &msg);
			SA_Log("Data Revc", LOG_NET, LOG_CLASS_DEBUG);
			failCount = 0;
		}
	}
}

static void SA_NetSetupServer(connectParam* parms)
{
	SA_WinNetOpen();
	SA_NetResolveHost(parms, AF_INET);
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		SA_Log("Socket failed!", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		SA_Log("Socket ok", LOG_NET, LOG_CLASS_INFO);
	}
	if (bind(parms->ctx->srvSocket, (SOCKADDR*)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr)) != 0)
	{
		SA_Log("Bind failed!", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		SA_Log("Bind ok", LOG_NET, LOG_CLASS_INFO);
	}
	if (listen(parms->ctx->srvSocket, 1) != 0)
	{
		SA_Log("Listen failed!", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		SA_Log("Listen ok", LOG_NET, LOG_CLASS_INFO);
	}
}

static unsigned short int SA_NetServerGetHandhake(connectParam* localParm)
{
	SA_Log("Waiting for client...", LOG_NET, LOG_CLASS_INFO);
	localParm->ctx->clientSocket = SOCKET_ERROR;
	localParm->dataSize = SA_DataGetDataFrameSize(localParm->dh);
	localParm->delay = SA_DataGetDelayInterFrames(localParm->dh);
	while ((int)localParm->ctx->clientSocket == SOCKET_ERROR && localParm->thread != NULL)
	{
		localParm->ctx->clientSocket = accept(localParm->ctx->srvSocket, NULL, NULL);
		SA_Sleep(localParm->delay);
	}
	SA_Log("Client connected! (1/3)", LOG_NET, LOG_CLASS_INFO);
	dataHandshake localDh = { 0xff, 0, 0, 0, 0, 0, 0, 0 };
	if (recv(localParm->ctx->clientSocket, (char*)&localDh, sizeof(dataHandshake), 0) == SOCKET_ERROR)
	{
		SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING);
	}
	else
	{
		SA_Log("Recived 'hello' code (2/3)", LOG_NET, LOG_CLASS_INFO);
		localParm->dh->header = HANDSHAKE;
		if (send(localParm->ctx->clientSocket, (char*)localParm->dh, sizeof(dataHandshake), 0) == SOCKET_ERROR)
		{
			SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
		}
		else
		{
			if (recv(localParm->ctx->clientSocket, data, DATASIZE + 1, 0) == SOCKET_ERROR)
			{
				SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING);
			}
			if (strcmp((char*)confirmConn, data) == 0)
			{
				SA_Log("Handshake complete (3/3)", LOG_NET, LOG_CLASS_INFO);
				return 1;
			}
		}
		SA_Log("Handshake failled", LOG_NET, LOG_CLASS_WARNING);
	}
	return 0;
}

static void SA_NetServer(void* parms)
{
	connectParam* localParm = (connectParam*)parms;
	while (localParm->thread != NULL)
	{
		while (!SA_NetServerGetHandhake((connectParam*)parms))
			;
		SA_Log("Audio connection established", LOG_NET, LOG_CLASS_INFO);
		int tolerance = 0;
		localParm->dh->sessionPacket = 0;
		time_t lastPacket = time(NULL);
		time_t lastTry;
		SA_ThreadCreate(SA_NetServerRecv, parms);
		data[DATASIZE + 2] = 0x00;
		while (1)
		{
			int delayed = 1;
			lastTry = time(NULL) - lastPacket;
			if (localParm->thread == NULL)
			{
				audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->dh, 1);
				dataHeader* header = (dataHeader*)audioDataFrame;
				*header = END;
				break;
			}
			if (lastTry > 10)
			{
				audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->dh, 1);
				dataHeader* header = (dataHeader*)audioDataFrame;
				*header = NULLDATA;
			}
			if (data[DATASIZE + 2] != 0x00 && localParm->thread != NULL)
			{
				audioDataFrame = SA_DataCreateDataFrame((float*)data, localParm->dh, 0);
				dataHeader* header = (dataHeader*)audioDataFrame;
				*header = DATAMSG;
				memcpy_s((char*)(header + 1), DATASIZE + 2, data, DATASIZE + 2);
				delayed = 0;
				SA_Log("Msg send", LOG_NET, LOG_CLASS_DEBUG);
			}
			if (audioDataFrame != NULL)
			{
				SA_DataPutOrderDataFrame((char*)audioDataFrame, localParm->dh->sessionPacket, localParm->dh);
				if (send(localParm->ctx->clientSocket, (char*)audioDataFrame, (int)localParm->dataSize, 0) == SOCKET_ERROR)
				{
					if (tolerance > 5)
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_DEBUG);
					tolerance++;
					SA_Sleep(1000);
					break;
				}
				else
				{
					data[DATASIZE + 2] = 0x00;
					lastPacket = time(NULL);
					localParm->dh->sessionPacket++;
					tolerance = 0;
				}
				audioDataFrame != NULL ? free(audioDataFrame) : (void)0;
				audioDataFrame = NULL;
			}
			if (delayed) {
				SA_Sleep(localParm->delay);
			}

		}
		localParm->ctx->clientSocket = 0;
	}
	SA_WinNetEnd(parms);
}

static unsigned short int SA_NetSetupClient(connectParam* parms)
{
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		SA_Log("Socket failed!", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		SA_Log("Socket ok", LOG_NET, LOG_CLASS_INFO);
		if (head != NULL)
		{
			if (head->next == NULL)
			{
				free(head->data);
				free(head);
			}
			else
			{
				for (audioBuffer* i = head; i != NULL; i = i->next)
				{
					free(i->data);
					if (i->next == NULL)
					{
						free(i);
						break;
					}
					else
					{
						free(i->prev);
					}
				}
			}
			head = NULL;
		}
		SA_NetResolveHost(parms, AF_INET);
		SA_Log("Connecting...", LOG_NET, LOG_CLASS_INFO);
		if (connect(parms->ctx->srvSocket, (SOCKADDR*)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr)) != 0)
		{
			SA_Log("Connect failed!", LOG_NET, LOG_CLASS_WARNING);
		}
		else
		{
			SA_Log("Host connected, sending 'hello' code (1/3)", LOG_NET, LOG_CLASS_INFO);
			if (send(parms->ctx->srvSocket, (char*)parms->dh, sizeof(*parms->dh), 0) == SOCKET_ERROR)
			{
				SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
			}
			else
			{
				SA_Log("Hello send... Wait Handshake (2/3)", LOG_NET, LOG_CLASS_INFO);
				if (recv(parms->ctx->srvSocket, (char*)parms->dh, sizeof(*parms->dh), 0) == SOCKET_ERROR)
				{
					SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING);
				}
				else
				{
					if (send(parms->ctx->srvSocket, (char*)confirmConn, 2, 0) == SOCKET_ERROR)
					{
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
					}
					else
					{
						SA_Log("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO);
						parms->dataSize = SA_DataGetDataFrameSize(parms->dh);
						SA_Log("Audio connection established", LOG_NET, LOG_CLASS_INFO);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static void SA_NetClientSend(connectParam* parms)
{
	data[DATASIZE + 2] = 0x00;
	size_t count = 0;
	while (parms->thread != NULL && parms->ctx->clientSocket != 0) {
		if (data[DATASIZE + 2] != 0x00) {
			if (send(parms->ctx->srvSocket, data, DATASIZE + 2, 0) == SOCKET_ERROR)
			{
				SA_Log("Msg failed!", LOG_NET, LOG_CLASS_DEBUG);
				if (count > 10) {
					break;
					SA_Log("Msg failed!", LOG_NET, LOG_CLASS_WARNING);
				}
				count++;
			}
			else
			{
				SA_Log("Msg send!", LOG_NET, LOG_CLASS_DEBUG);
				data[DATASIZE + 2] = 0x00;
			}
		}
		SA_Sleep(10);
	}
}

static void SA_NetClient(void* parms)
{
	connectParam* localParm = (connectParam*)parms;
	PaStream* stream = NULL;
	char* localData = NULL;
	size_t chunckWaveSize = 0;
	size_t err = 0;
	char* msgStream = NULL;
	int rounds = 0;
	SA_WinNetOpen();
	while (localParm->thread != NULL)
	{
		while (localParm->thread != NULL && !(SA_NetSetupClient(localParm)))
			;
		localData = malloc(localParm->dataSize);
		if (localData)
		{
			stream = SA_AudioOpenStream(localParm->device, 0, (void*)localParm->dh);
			SA_AudioStartStream(stream);
			void* sendThread = SA_ThreadCreate(SA_NetClientSend, localParm);
			while (localParm->thread != NULL && localParm->dataSize > sizeof(dataHandshake) + sizeof(size_t) * 2)
			{
				if (recv(localParm->ctx->srvSocket, (char*)localData, (int)localParm->dataSize, MSG_WAITALL) == SOCKET_ERROR)
				{
					err++;
					if (err > 10)
					{
						SA_Log("Close connection", LOG_NET, LOG_CLASS_WARNING);
						SA_AudioCloseStream(stream);
						stream = NULL;
						free(localData);
						localData = NULL;
						stream = NULL;
						break;
					}
				}
				else
				{
					localParm->dh->totalPacketSrv = SA_DataGetOrderDataFrame(localData, localParm->dh);
					err = 0;
					dataHeader* header = (dataHeader*)localData;
					switch (header[0])
					{
					case DATA:
						chunckWaveSize = SA_DataGetWaveSize(localData);
						if (chunckWaveSize != localParm->dh->waveSize * (size_t)localParm->dh->channel)
						{
							SA_Log("Invalid data size", LOG_NET, LOG_CLASS_WARNING);
							break;
						}
						audioDataFrame = malloc(localParm->dataSize);
						audioDataFrame == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
						if (audioDataFrame != NULL)
						{
							memcpy_s(audioDataFrame, localParm->dataSize, localData, localParm->dataSize);
							audioBuffer* temp = (audioBuffer*)malloc(sizeof(audioBuffer));
							if (temp)
							{
								temp->data = audioDataFrame;
								temp->next = NULL;
								temp->prev = NULL;
								if (head == NULL)
								{
									head = temp;
								}
								else
								{
									int bufferSize = 0;
									int delete = 0;
									for (audioBuffer* i = head; i != NULL; i = i->next)
									{
										bufferSize++;
										if (bufferSize > 5)
										{
											free(i->data);
											if (delete == 0)
											{
												i->prev->next = temp;
												temp->prev = i->prev;
												delete++;
											}
											else if (i->next != NULL)
											{
												free(i->prev);
											}
											else
											{
												free(i->prev);
												free(i);
												break;
											}
										}
										else if (i->next == NULL)
										{
											i->next = temp;
											temp->prev = i;
											break;
										}
									}
								}
							}
							else
							{
								SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
							}
						}
						chunckWaveSize = 0;
						localParm->dh->sessionPacket++;
						break;
					case DATAMSG:
						SA_DataRevcProcess(&rounds, &msgStream, (char*)(header + 1), &msg);
						SA_Log("Msg revc!", LOG_NET, LOG_CLASS_DEBUG);
						break;
					case END:
						SA_Log("Connection closed", LOG_NET, LOG_CLASS_INFO);
						closesocket(localParm->ctx->srvSocket);
						SA_AudioCloseStream(stream);
						stream = NULL;
						break;
					case NULLDATA:
						break;
					default:
						SA_Log("Invalid header", LOG_NET, LOG_CLASS_WARNING);
						break;
					}
				}
			}
			if (stream != NULL)
			{
				SA_Sleep(500); //UNSECURE
				SA_AudioCloseStream(stream);
			}
			SA_ThreadClose(sendThread);
			localParm->ctx->srvSocket = 0;
			free(localData);
		}
		else
		{
			SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
		}
		closesocket(localParm->ctx->srvSocket);
	}
	SA_WinNetEnd(parms);
}

void* SA_NetInit(int port, const char* host, int asClient, int device, dataHandshake* dh)
{
	void* netThread;
	connectParam* dataParm = NULL;
	dataParm = malloc(sizeof(connectParam));
	if (dataParm != NULL)
	{
		dataParm->ctx = malloc(sizeof(netCtx));
		dataParm->ctx == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
		dataParm->host = host;
		dataParm->port = port;
		dataParm->device = device;
		dataParm->dh = dh;
		if (asClient)
		{
			dataParm->asServer = 0;
			netThread = SA_ThreadCreate(SA_NetClient, (void*)dataParm);
		}
		else
		{
			dataParm->asServer = 1;
			SA_NetSetupServer(dataParm);
			netThread = SA_ThreadCreate(SA_NetServer, (void*)dataParm);
		}
		netThread == NULL ? SA_Log("Failed to create thread", LOG_NET, LOG_CLASS_ERROR) : (void)0;
		dataParm->thread = netThread;
		return &dataParm->thread;
	}
	else
	{
		SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
	}
	return NULL;
}

void SA_NetClose(void* thread)
{
	SA_ThreadClose(thread);
}
