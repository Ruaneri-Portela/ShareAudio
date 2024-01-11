#include "audio.h"
#include "data.h"
#include "log.h"
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

typedef struct connectParam
{
	int port;
	char addr[32];
	char* host;
} connectParam;

size_t runNet = 1;

static connectParam* parm = NULL;

static char data[1028];
static int dataSize = 0;
static const char confirmConn[] = { 0xFF, '\0' };

static void inetSrv()
{
	int iResult;
	WSADATA wsaData;
	SOCKET clientSocket;
	SOCKADDR_IN srvAddr;
	SOCKET srvSocket;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		logCat("Winsock2 failled!", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	else
	{
		logCat("WSA Start", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}

	srvAddr.sin_port = htons(parm->port);
	srvAddr.sin_family = AF_INET;
	inet_pton(AF_INET, parm->addr, &srvAddr.sin_addr);
	srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (srvSocket == INVALID_SOCKET)
	{
		logCat("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	iResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(srvAddr));
	if (iResult != 0)
	{
		logCat("Bind failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Bind ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}

	iResult = listen(srvSocket, 1);
	if (iResult != 0)
	{
		logCat("Listen failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Listen ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	dh = malloc(sizeof(dataHandshake));
	memset(dh, 0, sizeof(dataHandshake));
	if (dh != NULL) {
		dh->header = NULLHEADER;
	}
	else
	{
		logCat("Malloc failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	int connectSession = 1;
	int serverSession = 1;
	while (runNet)
	{
		connectSession = 1;
		logCat("Waiting for client...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		clientSocket = SOCKET_ERROR;
		while (clientSocket == SOCKET_ERROR)
		{
			clientSocket = accept(srvSocket, NULL, NULL);
		}
		logCat("Client connected!", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		while (connectSession && runNet)
		{
			while (connectSession && runNet)
			{
				iResult = recv(clientSocket, (char*)dh, sizeof(dataHandshake), 0);
				if (iResult == SOCKET_ERROR)
				{
					logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					connectSession = 0;
					break;
				}
				else
				{
					logCat("Recv ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					dh->header = HANDSHAKE;
					dh->channel = 2;
					dh->sampleRate = 44100;
					dh->waveSize = 1028;
					iResult = send(clientSocket, (char*)dh, sizeof(dataHandshake), 0);
					if (iResult == SOCKET_ERROR)
					{
						logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						connectSession = 0;
						break;
					}
					else
					{
						iResult = recv(clientSocket, data, 1028, 0);
						if (iResult == SOCKET_ERROR)
						{
							logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
							connectSession = 0;
							break;
						}
						else
						{
							if (strcmp(data, confirmConn))
							{
								logCat("Handshake ok!", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
							}
							break;
						}
					}
					logCat("Handshake failled", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					connectSession = 0;
					break;
				}
			}
			dataSize = (int)(sizeof(dataHeader) + sizeof(size_t) + ((sizeof(float) * dh->waveSize) * dh->channel) * dh->channel);
			logCat("Audio connection stablished", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
			size_t delay = (size_t)(((((float)dh->waveSize * (float)dh->channel) / (float)dh->sampleRate) / 4.0) * 1000);
			while (connectSession && runNet)
			{
				if (audioDataFrame != NULL)
				{
					iResult = send(clientSocket, (char*)audioDataFrame, dataSize, 0);
					if (iResult == SOCKET_ERROR)
					{
						logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						connectSession = 0;
						break;
					}
					free(audioDataFrame);
					audioDataFrame = NULL;
				}
				Sleep((DWORD)delay);
			};
		}
	}
	iResult = WSACleanup();
	if (iResult != 0)
	{
		logCat("WSA End failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
	}
	else
	{
		logCat("WSA End", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	free(parm);
}

static void inetClient()
{
	WSADATA wsaData;
	SOCKET remoteSocket;
	SOCKADDR_IN remoteAddr;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		logCat("Winsock2 failled!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("WSA Start", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
connect:
	remoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (remoteSocket == INVALID_SOCKET)
	{
		logCat("Socket failed!", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		logCat("Socket ok", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	}
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(parm->port);
	inet_pton(AF_INET, parm->addr, &remoteAddr.sin_addr);
	logCat("Connecting...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	iResult = connect(remoteSocket, (SOCKADDR*)&remoteAddr, sizeof(remoteAddr));
	if (iResult != 0)
	{
		logCat("Connect failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
		Sleep(1000);
		goto connect;
	}
	else
	{
		logCat("Host connected, sending 'hello' (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		dh = malloc(sizeof(dataHandshake));
		memset(dh, 0, sizeof(dataHandshake));
		dh->header = NULLHEADER;
		size_t err = 0;
		startStream(globalStream);
		while (runNet)
		{
			if (dh->header == NULLHEADER)
			{
				iResult = send(remoteSocket, (char*)dh, sizeof(dataHandshake), 0);
				if (iResult == SOCKET_ERROR)
				{
					logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
				}
				else
				{
					logCat("Hello send... Wait Handshake (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					iResult = recv(remoteSocket, (char*)dh, sizeof(dataHandshake), 0);
					if (iResult == SOCKET_ERROR)
					{
						logCat("Recv failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					}
					else
					{
						send(remoteSocket, confirmConn, (int)strlen(confirmConn), 0);
						if (iResult == SOCKET_ERROR)
						{
							logCat("Send failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						}
						else
						{
							dataHandshake* test = dh;
							logCat("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
							dataSize = (int)(sizeof(dataHeader) + sizeof(size_t) + ((sizeof(float) * dh->waveSize) * dh->channel) * dh->channel);
						}
					}
				}
			}
			else if (dataSize > sizeof(dataHeader) + sizeof(size_t))
			{
				void* localData = malloc(dataSize);
				memset(localData, 0, dataSize);
				iResult = recv(remoteSocket, (char*)localData, dataSize, 0);
				if (iResult == SOCKET_ERROR)
				{
					err++;
					logCat("Client cannot recive data", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
					free(localData);
					head = NULL;
					if (err > 10)
					{
						logCat("Close connection", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
						closesocket(remoteSocket);
						free(dh);
						stopStream(globalStream);
						goto connect;
					}
				}
				else
				{
					err = 0;
					audioDataFrame = localData;
					dataHeader* header = (dataHeader*)audioDataFrame;
					size_t* waveSize = (size_t*)(header + sizeof(dataHeader));
					float* data = (float*)(waveSize + sizeof(size_t));
					switch (header[0])
					{
					case DATA:
						audioBuffer* temp = (audioBuffer*)malloc(sizeof(audioBuffer));
						temp->data = audioDataFrame;
						temp->next = NULL;
						if (head == NULL)
						{
							head = temp;
						}
						else
						{
							if (audioDataFrame != NULL)
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
						}
						break;
					case HANDSHAKE:
						break;
					}
				}
			}
		}
		closesocket(remoteSocket);
		iResult = WSACleanup();
		if (iResult != 0)
		{
			logCat("WSA End failed!", LOG_NET, LOG_CLASS_WARNING, logOutputMethod);
		}
		else
		{
			logCat("WSA End", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		}
	}
}

HANDLE initNet(int port, char addr[], char* host, size_t asClient)
{
	HANDLE hThread;
	parm = malloc(sizeof(connectParam));
	parm->host = host;
	parm->port = port;
	strcpy_s(parm->addr, 32, addr);
	DWORD dwThreadId;
	if (asClient)
	{
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inetClient, NULL, 0, &dwThreadId);
	}
	else
	{
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inetSrv, NULL, 0, &dwThreadId);
	}

	if (hThread == NULL)
	{
		printf("Failed to create thread\n");
		return NULL;
	}
	return hThread;
}

void closeNet(void* hThread)
{
	runNet = 0;
	WaitForSingleObject((HANDLE*)hThread, INFINITE);
	CloseHandle((HANDLE*)hThread);
	free(parm);
}
