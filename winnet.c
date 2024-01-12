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

static connectParam* parm = NULL;

static char data[1028];
static int dataSize = 0;
static const unsigned char confirmConn[3] = { 0xFF, '\0' };

HANDLE closeThread = NULL;

static void closeApplication()
{
	getchar();
	stopStream(globalStream);
	HANDLE local = closeThread;
	closeThread = NULL;
	CloseHandle((HANDLE*)local);
}

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
	int connectSession = 1;
	while (closeThread != NULL)
	{
		connectSession = 1;
		logCat("Waiting for client...", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		clientSocket = SOCKET_ERROR;
		while ((int)clientSocket == SOCKET_ERROR)
		{
			clientSocket = accept(srvSocket, NULL, NULL);
		}
		logCat("Client connected! (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		while (connectSession && closeThread != NULL)
		{
			while (connectSession && closeThread != NULL)
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
					logCat("Recived 'hello' code (2/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
					dh->header = HANDSHAKE;
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
							if (data[0] == confirmConn[0])
							{
								logCat("Handshake complet (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
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
					iResult = send(clientSocket, (char*)audioDataFrame, dataSize, 0);
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
		logCat("Host connected, sending 'hello' code (1/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
		dh->header = NULLHEADER;
		size_t err = 0;
		PaStream* globalStream = NULL;
		while (closeThread != NULL)
		{
			if (dh->header == NULLHEADER && globalStream == NULL)
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
							logCat("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
							dataSize = (int)(sizeof(dataHeader) + sizeof(size_t) + ((sizeof(float) * dh->waveSize) * dh->channel) * dh->channel);
						}
					}
				}
				globalStream = setupStream(deviceAudio, 2, dh->sampleRate, dh->waveSize, 0);
				startStream(globalStream);
			}
			else if (dataSize > (int)(sizeof(dataHeader) + sizeof(size_t)))
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
						shutdownStream(globalStream);
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
					case END:
						logCat("Connection closed", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
						closesocket(remoteSocket);
						shutdownStream(globalStream);
						globalStream = NULL;
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
		}
		if (globalStream != NULL)
			shutdownStream(globalStream);
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
	closeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)closeApplication, NULL, 0, NULL);
	if (closeThread == NULL)
	{
		printf("Failed to create thread\n");
		return NULL;
	}

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
	WaitForSingleObject((HANDLE*)hThread, INFINITE);
	CloseHandle((HANDLE*)hThread);
	free(parm);
}
