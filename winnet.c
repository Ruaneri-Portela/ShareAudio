#include "audio.h"
#include "data.h"
#include "log.h"
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

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

static char data[1028];
static const unsigned char confirmConn[3] = { 0xFF, '\0' };

HANDLE closeThread = NULL;

static int iResult;

static DWORD WINAPI closeApplication()
{
	getchar();
	stopStream(globalStream);
	HANDLE local = closeThread;
	closeThread = NULL;
	CloseHandle((HANDLE*)local);
	return 0;
}

static void closeInet(LPVOID parms)
{
	free(parms);
	int iResult = WSACleanup();
	if (iResult != 0)
	{
		logCat("WSA End failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
	}
	else
	{
		logCat("WSA End", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}

static void initInetCommon() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		logCat("Winsock2 failled!", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	else
	{
		logCat("WSA Start", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}


static void setupAddr(connectParam* parm, ADDRESS_FAMILY family) {
	int detectIp = detectHost(parm->host, parm->asServer);
	char* hostLocal = parm->host;
	struct addrinfo* res = NULL;
	char* ip = NULL;
	if (detectIp == 2) {
		hostLocal = NULL;
		int tam = sizeof(char) * 16;
		char* ip = malloc(tam);
		if (ip == NULL)
			logCat("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
		getaddrinfo(parm->host, 0, 0, &res);
		for (struct addrinfo* ptr = res; ptr != NULL; ptr = ptr->ai_next) {
			if (ptr->ai_addr->sa_family == AF_INET) {
				struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
				inet_ntop(AF_INET, &(ipv4->sin_addr), ip, 16);
				hostLocal = ip;
			}
		}
	}
	parm->ctx->srvAddr.sin_family = family;
	parm->ctx->srvAddr.sin_port = htons(parm->port);
	if (inet_pton(AF_INET, hostLocal, &parm->ctx->srvAddr.sin_addr, strlen(parm->host)) == 0 && hostLocal != NULL) {
		if (res != NULL) {
			free(res);
		}
		if (ip != NULL) {
			free(ip);
		}
		logCat("Invalid address", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		const char* msgPrefix;
		if (parm->asServer == 1) {
			msgPrefix = "Bind on:";
		}
		else {
			msgPrefix = "Connect to host:";
		}
		size_t tamanhoMsg = strlen(msgPrefix) + strlen(parm->host) + 1;
		char* msg = malloc(sizeof(char) * tamanhoMsg);
		if (msg != NULL) {
			strcpy_s(msg, tamanhoMsg, msgPrefix);
			strcat_s(msg, tamanhoMsg, parm->host);
			logCat(msg, LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		}
		else
		{
			logCat("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
		}
	}
}

static void setupSrv(connectParam* parms) {
	initInetCommon();
	setupAddr(parms, AF_INET);
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		logCat("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	iResult = bind(parms->ctx->srvSocket, (SOCKADDR*)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr));
	if (iResult != 0)
	{
		logCat("Bind failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Bind ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}

	iResult = listen(parms->ctx->srvSocket, 1);
	if (iResult != 0)
	{
		logCat("Listen failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Listen ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}

static short unsigned int inetSrvHandshake(connectParam* localParm) {
	short unsigned int connectSession = 1;
	logCat("Waiting for client...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	localParm->ctx->clientSocket = SOCKET_ERROR;
	localParm->dataSize = getSize(dh);
	localParm->delay = getDelay(dh);
	while ((int)localParm->ctx->clientSocket == SOCKET_ERROR && closeThread != NULL)
	{
		localParm->ctx->clientSocket = accept(localParm->ctx->srvSocket, NULL, NULL);
		Sleep((DWORD)localParm->delay);
	}
	logCat("Client connected! (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	while (connectSession && closeThread != NULL)
	{
		iResult = recv(localParm->ctx->clientSocket, (char*)dh, sizeof(dataHandshake), 0);
		if (iResult == SOCKET_ERROR)
		{
			logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
			connectSession = 0;
			break;
		}
		else
		{
			logCat("Recived 'hello' code (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
			dh->header = HANDSHAKE;
			iResult = send(localParm->ctx->clientSocket, (char*)dh, sizeof(dataHandshake), 0);
			if (iResult == SOCKET_ERROR)
			{
				logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
				connectSession = 0;
				break;
			}
			else
			{
				iResult = recv(localParm->ctx->clientSocket, data, 1028, 0);
				if (iResult == SOCKET_ERROR)
				{
					logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					connectSession = 0;
					break;
				}
				if (strcmp((char*)confirmConn, data) == 0)
				{
					logCat("Handshake complete (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					return 1;
				}
			}
			logCat("Handshake failled", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
			connectSession = 0;
			break;
		}
		Sleep((DWORD)localParm->delay);
	}
	return connectSession;
}

static DWORD WINAPI inetSrv(LPVOID parms)
{
	connectParam* localParm = (connectParam*)parms;
	int connectSession = 1;
	while (closeThread != NULL)
	{
		connectSession = 1;
		while (!inetSrvHandshake((connectParam*)parms));
		logCat("Audio connection stablished", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		int tolerance = 0;
		while (connectSession)
		{
			if (closeThread == NULL)
			{
				audioDataFrame = createDataFrame(NULL, dh->waveSize);
				dataHeader* header = (dataHeader*)audioDataFrame;
				*header = END;
				connectSession = 0;
			}
			if (audioDataFrame != NULL)
			{
				iResult = send(localParm->ctx->clientSocket, (char*)audioDataFrame, (int)localParm->dataSize, 0);
				if (iResult == SOCKET_ERROR)
				{
					logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					connectSession = 0;
					tolerance++;
					if (tolerance > 10)
						break;
				}
				else
				{
					tolerance = 0;
				}
				free(audioDataFrame);
				audioDataFrame = NULL;
			}
			Sleep((DWORD)localParm->delay);
		};
	}
	closeInet(parms);
	return 0;
}

static DWORD WINAPI inetClient(LPVOID parms)
{
	connectParam localParm = *(connectParam*)parms;
	initInetCommon();
connect:
	localParm.ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (localParm.ctx->srvSocket == INVALID_SOCKET)
	{
		logCat("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	setupAddr(&localParm, AF_INET);
	logCat("Connecting...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	iResult = connect(localParm.ctx->srvSocket, (SOCKADDR*)&localParm.ctx->srvAddr, sizeof(localParm.ctx->srvAddr));
	if (iResult != 0)
	{
		logCat("Connect failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
		goto connect;
	}
	else
	{
		logCat("Host connected, sending 'hello' code (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		dh->header = NULLHEADER;
		size_t err = 0;
		PaStream* globalStream = NULL;
		while (closeThread != NULL)
		{
			if (dh->header == NULLHEADER && globalStream == NULL)
			{
				iResult = send(localParm.ctx->srvSocket, (char*)dh, sizeof(dataHandshake), 0);
				if (iResult == SOCKET_ERROR)
				{
					logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
				}
				else
				{
					logCat("Hello send... Wait Handshake (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					iResult = recv(localParm.ctx->srvSocket, (char*)dh, sizeof(dataHandshake), 0);
					if (iResult == SOCKET_ERROR)
					{
						logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					}
					else
					{
						send(localParm.ctx->srvSocket, (char*)confirmConn, 3, 0);
						if (iResult == SOCKET_ERROR)
						{
							logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						}
						else
						{
							logCat("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
							localParm.dataSize = getSize(dh);
						}
					}
				}
				globalStream = setupStream(deviceAudio, 2, dh->sampleRate, dh->waveSize, 0);
				startStream(globalStream);
			}
			else if (localParm.dataSize > (int)(sizeof(dataHeader) + sizeof(size_t)))
			{
				void* localData = malloc(localParm.dataSize);
				if (localData != NULL) {
					memset(localData, 0, localParm.dataSize);
					iResult = recv(localParm.ctx->srvSocket, (char*)localData, localParm.dataSize, 0);
					if (iResult == SOCKET_ERROR)
					{
						err++;
						logCat("Client cannot recive data", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						free(localData);
						head = NULL;
						if (err > 10)
						{
							logCat("Close connection", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
							if (globalStream != NULL) {
								shutdownStream(globalStream);
								globalStream = NULL;
							};
							globalStream = NULL;
							goto connect;
						}
					}
					else
					{
						err = 0;
						audioDataFrame = localData;
						dataHeader* header = (dataHeader*)audioDataFrame;
						switch (header[0])
						{
						case DATA:
							if (audioDataFrame != NULL)
							{
								audioBuffer* temp = (audioBuffer*)malloc(sizeof(audioBuffer));
								temp->data = audioDataFrame;
								temp->next = NULL;
								if (head == NULL)
								{
									head = temp;
								}
								else
								{
									for (audioBuffer* i = head; i != NULL; i = i->next)
									{
										if (i->next == NULL)
										{
											i->next = temp;
											break;
										}
									}
								}
								audioDataFrame = NULL;
							}
							break;
						case END:
							logCat("Connection closed", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
							closesocket(localParm.ctx->srvSocket);
							if (globalStream != NULL) {
								shutdownStream(globalStream);
								globalStream = NULL;
							}
							goto connect;
						case AUTH:
							break;
						case HANDSHAKE:
							break;
						default:
							break;
						}
					}
				}
				else
				{
					logCat("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
				}
			}
		}
		if (globalStream != NULL) {
			shutdownStream(globalStream);
			globalStream = NULL;
		}
		closesocket(localParm.ctx->srvSocket);
		closeInet(parms);
	}
	return 0;
}

HANDLE initNet(int port, char* host, size_t asClient)
{
	closeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)closeApplication, NULL, 0, NULL);
	if (closeThread == NULL)
	{
		printf("Failed to create thread\n");
		return NULL;
	}
	HANDLE hThread;
	connectParam* dataParm = NULL;
	dataParm = malloc(sizeof(connectParam));
	if (dataParm != NULL)
	{
		dataParm->ctx = malloc(sizeof(srvCtx));
		if (dataParm->ctx == NULL)
		{
			logCat("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
		}
		dataParm->host = host;
		dataParm->port = port;
		if (asClient)
		{
			dataParm->asServer = 0;
			hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inetClient, dataParm, 0, NULL);
		}
		else
		{
			dataParm->asServer = 1;
			setupSrv(dataParm);
			hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inetSrv, dataParm, 0, NULL);
		}
		if (hThread == NULL)
		{
			printf("Failed to create thread\n");
			return NULL;
		}
		return hThread;
	}
	else {
		logCat("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	return NULL;
}

void closeNet(void* hThread)
{
	WaitForSingleObject((HANDLE*)hThread, INFINITE);
	CloseHandle((HANDLE*)hThread);
}
