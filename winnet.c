#include "audio.h"
#include "data.h"
#include "log.h"
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

typedef struct srvCtx
{
	SOCKET clientSocket;
	SOCKET srvSocket;
	SOCKADDR_IN srvAddr;
} srvCtx;

typedef struct connectParam
{
	unsigned short int asServer;
	unsigned int port;
	char *host;
	size_t dataSize;
	short int device;
	size_t delay;
	srvCtx *ctx;
} connectParam;

static char data[1028];

static const unsigned char confirmConn[2] = {0xFF, '\0'};

static WSADATA wsaData;

size_t sessionPacket = 0;

size_t totalPacketSrv = 0;

HANDLE closeThread = NULL;

static DWORD WINAPI SA_Console()
{
	char comand[1028];
	Sleep(1000);
	while (1)
	{
		printf_s("Comand => ");
		scanf_s("%s", comand, 1028);
		if (strcmp(comand, "exit") == 0)
		{
			HANDLE local = closeThread;
			CloseHandle((HANDLE *)local);
			closeThread = NULL;
			break;
		}
		else if (strcmp(comand, "info") == 0)
		{
			printf_s("Session packet: %zu\n", sessionPacket);
			if (totalPacketSrv != 0)
			{
				printf_s("Total packet: %zu\n", totalPacketSrv + 1);
				printf_s("Packet lost: %zu\n", (totalPacketSrv + 1) - sessionPacket);
				printf_s("Packet lost percent: %.2f\n", ((float)((totalPacketSrv + 1) - sessionPacket)) / (float)totalPacketSrv * 100);
				double totalMB = ((double)(sizeof(size_t) * dh->waveSize * dh->channel * totalPacketSrv) / 1024.00f) / 1024.00f;
				printf_s("Total MB reviced: %.2lf\n", totalMB);
			}
		}
		else if (strcmp(comand, "help") == 0)
		{
			printf_s("exit => Close application\n");
			printf_s("info => Show info\n");
			printf_s("volume => Set volume\n");
		}
		else if (strcmp(comand, "volume") == 0)
		{
			printf_s("New volume value: ");
			scanf_s("%f", &volMod);
		}
		else
		{
			printf_s("Invalid command\n");
		}
	}
	return 0;
}

static void SA_WinNetEnd(LPVOID parms)
{
	free(((connectParam *)parms)->ctx);
	free(parms);
	if (WSACleanup() != 0)
	{
		SA_Log("WSA End failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
	}
	else
	{
		SA_Log("WSA End", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}

static void SA_WinNetOpen()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		SA_Log("Winsock2 failled!", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	else
	{
		SA_Log("WSA Start", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}

static void SA_NetResolveHost(connectParam *parm, ADDRESS_FAMILY family)
{
	unsigned short int detectIp = SA_DataDetectIsIp(parm->host, parm->asServer);
	switch (detectIp)
	{
	case 0:
		SA_Log("Try Resolve by host", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		break;
	case 1:
		SA_Log("IPv4 Detected", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		break;
	case 2:
		SA_Log("IP Forbbiden on Client", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
		break;
	}
	char *hostLocal = parm->host;
	struct addrinfo *res = NULL;
	char *ip = NULL;
	if (detectIp == 0)
	{
		hostLocal = NULL;
		int tam = sizeof(char) * 16;
		char *ip = malloc(tam);
		ip == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
		getaddrinfo(parm->host, 0, 0, &res);
		for (struct addrinfo *ptr = res; ptr != NULL; ptr = ptr->ai_next)
		{
			if (ptr->ai_addr->sa_family == AF_INET)
			{
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
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
		SA_Log("Invalid host address", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		const char *msgPrefix;
		if (parm->asServer == 1)
		{
			msgPrefix = "Listen on: ";
		}
		else
		{
			msgPrefix = "Connect to host: ";
		}
		char *logMsg = SA_DataConcatString(msgPrefix, parm->host);
		SA_Log(logMsg, LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		free(logMsg);
	}
}

static void SA_NetSetupServer(connectParam *parms)
{
	SA_WinNetOpen();
	SA_NetResolveHost(parms, AF_INET);
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		SA_Log("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		SA_Log("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	if (bind(parms->ctx->srvSocket, (SOCKADDR *)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr)) != 0)
	{
		SA_Log("Bind failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		SA_Log("Bind ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	if (listen(parms->ctx->srvSocket, 1) != 0)
	{
		SA_Log("Listen failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		SA_Log("Listen ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
}

static unsigned short int SA_NetServerGetHandhake(connectParam *localParm)
{
	SA_Log("Waiting for client...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	localParm->ctx->clientSocket = SOCKET_ERROR;
	localParm->dataSize = SA_DataGetDataFrameSize(dh);
	localParm->delay = SA_DataGetDelayInterFrames(dh);
	while ((int)localParm->ctx->clientSocket == SOCKET_ERROR && closeThread != NULL)
	{
		localParm->ctx->clientSocket = accept(localParm->ctx->srvSocket, NULL, NULL);
		Sleep((DWORD)localParm->delay);
	}
	SA_Log("Client connected! (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	dataHandshake localDh = {0xff, 0, 0, 0};
	if (recv(localParm->ctx->clientSocket, (char *)&localDh, sizeof(dataHandshake), 0) == SOCKET_ERROR)
	{
		SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
	}
	else
	{
		SA_Log("Recived 'hello' code (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		dh->header = HANDSHAKE;
		if (send(localParm->ctx->clientSocket, (char *)dh, sizeof(dataHandshake), 0) == SOCKET_ERROR)
		{
			SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
		}
		else
		{
			if (recv(localParm->ctx->clientSocket, data, 1028, 0) == SOCKET_ERROR)
			{
				SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
			}
			if (strcmp((char *)confirmConn, data) == 0)
			{
				SA_Log("Handshake complete (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
				return 1;
			}
		}
		SA_Log("Handshake failled", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
	}
	return 0;
}

static DWORD WINAPI SA_NetServer(LPVOID parms)
{
	connectParam *localParm = (connectParam *)parms;
	while (closeThread != NULL)
	{
		while (!SA_NetServerGetHandhake((connectParam *)parms))
			;
		SA_Log("Audio connection established", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		int tolerance = 0;
		sessionPacket = 0;
		while (1)
		{
			if (closeThread == NULL)
			{
				audioDataFrame = SA_DataCreateDataFrame(NULL, dh, 1);
				dataHeader *header = (dataHeader *)audioDataFrame;
				*header = END;
				break;
			}
			if (audioDataFrame != NULL)
			{
				SA_DataPutOrderDataFrame((char *)audioDataFrame, sessionPacket, dh);
				if (send(localParm->ctx->clientSocket, (char *)audioDataFrame, (int)localParm->dataSize, 0) == SOCKET_ERROR)
				{
					tolerance++;
					Sleep(1000);
					if (tolerance > 5)
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					break;
				}
				else
				{
					sessionPacket++;
					tolerance = 0;
				}
				free(audioDataFrame);
				audioDataFrame = NULL;
			}
			Sleep((DWORD)localParm->delay);
		};
	}
	SA_WinNetEnd(parms);
	return 0;
}

static unsigned short int SA_NetSetupClient(connectParam *parms)
{
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		SA_Log("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		SA_Log("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		if (head != NULL)
		{
			if (head->next == NULL)
			{
				free(head->data);
				free(head);
			}
			else
			{
				for (audioBuffer *i = head; i != NULL; i = i->next)
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
		SA_Log("Connecting...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		if (connect(parms->ctx->srvSocket, (SOCKADDR *)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr)) != 0)
		{
			SA_Log("Connect failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
		}
		else
		{
			SA_Log("Host connected, sending 'hello' code (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
			memset(dh, 0, sizeof(dataHandshake));
			if (send(parms->ctx->srvSocket, (char *)dh, sizeof(*dh), 0) == SOCKET_ERROR)
			{
				SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
			}
			else
			{
				SA_Log("Hello send... Wait Handshake (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
				if (recv(parms->ctx->srvSocket, (char *)dh, sizeof(*dh), 0) == SOCKET_ERROR)
				{
					SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
				}
				else
				{
					if (send(parms->ctx->srvSocket, (char *)confirmConn, 2, 0) == SOCKET_ERROR)
					{
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					}
					else
					{
						SA_Log("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
						parms->dataSize = SA_DataGetDataFrameSize(dh);
						struct timeval timeout = {0, 0};
						timeout.tv_sec = 0;
						timeout.tv_usec = (long)SA_DataGetDelayInterFrames(dh);
						if (setsockopt(parms->ctx->srvSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == SOCKET_ERROR)
						{
							SA_Log("Failed to set socket timeout", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
						}
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static DWORD WINAPI SA_NetClient(LPVOID parms)
{
	connectParam localParm = *(connectParam *)parms;
	PaStream *stream = NULL;
	char *localData = NULL;
	size_t chunckWaveSize = 0;
	size_t err = 0;
	SA_WinNetOpen();
	while (closeThread != NULL)
	{
		while (closeThread != NULL && !(SA_NetSetupClient(&localParm)))
			;
		localData = malloc(localParm.dataSize);
		localData == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
		stream = SA_AudioOpenStream(localParm.device, dh->channel, dh->sampleRate, dh->waveSize, 0);
		SA_AudioStartStream(stream);
		while (closeThread != NULL && localParm.dataSize > sizeof(dataHandshake) + sizeof(size_t) * 2)
		{
			if (recv(localParm.ctx->srvSocket, (char *)localData, (int)localParm.dataSize, 0) == SOCKET_ERROR)
			{
				err++;
				if (err > 10)
				{
					SA_Log("Close connection", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
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
				totalPacketSrv = SA_DataGetOrderDataFrame(localData, dh);
				err = 0;
				dataHeader *header = (dataHeader *)localData;
				switch (header[0])
				{
				case DATA:
					chunckWaveSize = SA_DataGetWaveSize(localData);
					if (chunckWaveSize != dh->waveSize * dh->channel)
					{
						SA_Log("Invalid data size", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						break;
					}
					audioDataFrame = malloc(localParm.dataSize);
					audioDataFrame == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
					if (audioDataFrame != NULL)
					{
						memcpy_s(audioDataFrame, localParm.dataSize, localData, localParm.dataSize);
						audioBuffer *temp = (audioBuffer *)malloc(sizeof(audioBuffer));
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
							for (audioBuffer *i = head; i != NULL; i = i->next)
							{
								bufferSize++;
								if (bufferSize > 5)
								{
									free(i->data);
									if (delete == 0)
									{
										i->prev->next = temp;
										temp->prev = i->prev;
										delete ++;
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
					chunckWaveSize = 0;
					sessionPacket++;
					break;
				case END:
					SA_Log("Connection closed", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					closesocket(localParm.ctx->srvSocket);
					SA_AudioCloseStream(stream);
					stream = NULL;
					break;
				default:
					SA_Log("Invalid header", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					break;
				}
			}
		}
		free(localData);
	}
	if (stream != NULL)
	{
		SA_AudioCloseStream(stream);
	}
	closesocket(localParm.ctx->srvSocket);
	SA_WinNetEnd(parms);
	return 0;
}

HANDLE SA_NetInit(int port, char *host, size_t asClient, int device)
{
	closeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SA_Console, NULL, 0, NULL);
	closeThread == NULL ? SA_Log("Failed to create thread", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
	HANDLE hThread;
	connectParam *dataParm = NULL;
	dataParm = malloc(sizeof(connectParam));
	if (dataParm != NULL)
	{
		dataParm->ctx = malloc(sizeof(srvCtx));
		dataParm->ctx == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
		dataParm->host = host;
		dataParm->port = port;
		dataParm->device = device;
		if (asClient)
		{
			dataParm->asServer = 0;
			hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SA_NetClient, dataParm, 0, NULL);
		}
		else
		{
			dataParm->asServer = 1;
			SA_NetSetupServer(dataParm);
			hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SA_NetServer, dataParm, 0, NULL);
		}
		hThread == NULL ? SA_Log("Failed to create thread", LOG_NET, LOG_CLASS_ERROR, logOutputMethod) : (void)0;
		return hThread;
	}
	else
	{
		SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	return NULL;
}

void SA_NetClose(void *hThread)
{
	WaitForSingleObject((HANDLE *)hThread, INFINITE);
	CloseHandle((HANDLE *)hThread);
}
