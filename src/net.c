
#include "config.h"

#if defined(LINUX)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include <portaudio.h>
#include <openssl/evp.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wav.h"
#include "data.h"
#include "audio.h"
#include "log.h"
#include "threads.h"
#include "encrypt.h"

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

#if defined(WINDOWS)
static WSADATA wsaData;
static void SA_WinNetEnd(void* parms)
{
	if (WSACleanup() != 0)
	{
		SA_Log("WSA End failed!", LOG_NET, LOG_CLASS_WARNING);
	}
	else
	{
		SA_Log("WSA End", LOG_NET, LOG_CLASS_INFO);
	}
	free(((connectParam*)parms)->ctx);
	free(parms);
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
	unsigned short int detectIp = SA_DataDetectIsIp(parm->conn->host, parm->conn->mode);
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
	const char* hostLocal = parm->conn->host;
	struct addrinfo* res = NULL;
	char* ip = NULL;
	if (detectIp == 0)
	{
		hostLocal = NULL;
		int tam = sizeof(char) * 16;
		char* ip = malloc(tam);
		ip == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
		getaddrinfo(parm->conn->host, 0, 0, &res);
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
	parm->ctx->srvAddr.sin_port = htons(parm->conn->port);
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
		const char* msgPrefix = parm->conn->mode == 1 ? "Connect to host: " : "Listen on: ";
		char* logMsg = SA_DataConcatString(msgPrefix, parm->conn->host);
		SA_Log(logMsg, LOG_NET, LOG_CLASS_INFO);
		free(logMsg);
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
	localParm->dataSize = SA_DataGetDataFrameSize(localParm->conn->dh);
	localParm->delay = SA_DataGetDelayInterFrames(localParm->conn->dh);
	while ((int)localParm->ctx->clientSocket == SOCKET_ERROR && localParm->conn->thread != NULL)
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
		size_t sizeSend = sizeof(dataHandshake);
		char* sendData = (char*)localParm->conn->dh;
		if (localParm->conn->key[0] != 0) {
			memcpy_s(localParm->conn->dh->iv, 128, localDh.iv, 128);
			localParm->encryptCtx = SA_IniCtx();
			SA_PrepareEncryptCtx(localParm->encryptCtx, localParm->conn->key, localParm->conn->dh->iv);
			sizeSend = SA_GetEncriptySize(sizeSend);
			unsigned char * sendEncrypt = malloc(sizeSend);
			if(sendEncrypt == NULL) {
				SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
				return 0;
			}
			sizeSend = SA_EncryptData((unsigned char*)&localDh, sizeof(dataHandshake), sendEncrypt, localParm->encryptCtx);
			sendData = (char*)sendEncrypt;
		}
		SA_Log("Recived 'hello' code (2/3)", LOG_NET, LOG_CLASS_INFO);
		localParm->conn->dh->header = HANDSHAKE;
		if (send(localParm->ctx->clientSocket,sendData, (int)sizeSend, 0) == SOCKET_ERROR)
		{
			SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
		}
		else
		{
			if (recv(localParm->ctx->clientSocket, localParm->conn->data, DATASIZE + 2, 0) == SOCKET_ERROR)
			{
				SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING);
			}
			if (strcmp((char*)confirmConn, localParm->conn->data) == 0)
			{
				SA_Log("Handshake complete (3/3)", LOG_NET, LOG_CLASS_INFO);
				if (localParm->encryptCtx != NULL) 
				{
					localParm->conn->runCode = 3;
				}
				else 
				{ 
					localParm->conn->runCode = 2; 
				}
				return 1;
			}
		}
		SA_Log("Handshake failled", LOG_NET, LOG_CLASS_WARNING);
	}
	return 0;
}

static void SA_NetServerRecv(connectParam* parms) {
	size_t failCount = 0;
	size_t rounds = 0;
	char msgLocal[DATASIZE + 3];
	char* msgStream = NULL;
	size_t sizeData = DATASIZE + 2;
	if (parms->conn->runCode == 3) {
		sizeData = SA_GetEncriptySize(sizeData);
		parms->decryptCtx = SA_IniCtx();
		SA_PrepareDecryptCtx(parms->decryptCtx, parms->conn->key, parms->conn->dh->iv);
	}
	while (parms->conn->runCode != -1) {
		int socket = recv(parms->ctx->clientSocket, msgLocal, (int)sizeData, MSG_WAITALL);
		if (socket == SOCKET_ERROR)
		{
			SA_Log("Revc failed!", LOG_NET, LOG_CLASS_DEBUG);
			SA_Sleep(100);
			if (failCount > 10) {
				SA_Log("Revc failed!", LOG_NET, LOG_CLASS_WARNING);
			}
			failCount++;
		}
		else
		{
			int processed = 1;
			if (parms->conn->runCode == 3) {
				unsigned char* recvDecrypt = malloc(sizeData);
				if (recvDecrypt == NULL) {
					SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
				}
				else {
					processed = SA_DecryptData((unsigned char*)msgLocal, (int)sizeData, recvDecrypt, parms->decryptCtx);
					memcpy_s(msgLocal, DATASIZE + 2, recvDecrypt, DATASIZE + 2);
					free(recvDecrypt);
				}
			}
			if (processed) {
				switch (msgLocal[DATASIZE + 1]) {
				case 0x01:
					SA_DataRevcProcess(&rounds, &msgStream, msgLocal, &parms->conn->msg);
					SA_Log("Data Revc", LOG_NET, LOG_CLASS_DEBUG);
					break;
				case 0x02:
					switch(((dataHeader)msgLocal[0])) {
					case VOLUPDATE:
						SA_Log("Vol update", LOG_NET, LOG_CLASS_DEBUG);
						parms->conn->dh->volMod = *((float*)(msgLocal + sizeof(dataHeader)));
						break;
					default:
						break;
					}
					break;
				}
				failCount = 0;
			}
			else {
				SA_Sleep(500);
			}
		}
	}
}

static void SA_NetServer(void* parms)
{
	connectParam* localParm = (connectParam*)parms;
	while (localParm->conn->runCode != -1)
	{
		localParm->conn->dh->sessionPacket = 0;
		localParm->conn->runCode = 1;
		while (!SA_NetServerGetHandhake((connectParam*)parms))
			;
		SA_Log("Audio connection established", LOG_NET, LOG_CLASS_INFO);
		int tolerance = 0;
		time_t lastPacket = time(NULL);
		time_t lastTry;
		int exitSender = 0;
		int isCrypt = localParm->conn->runCode == 3;

		void* revcThread = SA_ThreadCreate(SA_NetServerRecv, parms);
		localParm->conn->data[DATASIZE + 2] = 0x00;
		float volMod = localParm->conn->dh->volMod;
		while (1)
		{
			int delayed = 1;
			lastTry = time(NULL) - lastPacket;
			if (localParm->conn->runCode == -1)
			{
				localParm->conn->audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->conn->dh, 1);
				dataHeader* header = (dataHeader*)localParm->conn->audioDataFrame;
				*header = END;
				exitSender = 1;
			}
			if (lastTry > 10)
			{
				localParm->conn->audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->conn->dh, 1);
				dataHeader* header = (dataHeader*)localParm->conn->audioDataFrame;
				*header = NULLDATA;
			}
			if (localParm->conn->data[DATASIZE + 2] != 0x00)
			{
				localParm->conn->audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->conn->dh, 1);
				dataHeader* header = (dataHeader*)localParm->conn->audioDataFrame;
				*header = DATAMSG;
				memcpy_s((char*)(header + 1), DATASIZE + 2, localParm->conn->data, DATASIZE + 2);
				delayed = 0;
			}
			if (localParm->conn->dh->volMod != volMod) {
				localParm->conn->audioDataFrame = SA_DataCreateDataFrame(NULL, localParm->conn->dh, 1);
				dataHeader* header = (dataHeader*)localParm->conn->audioDataFrame;
				*header = VOLUPDATE;
				float* volModUpdate = (float*)(header + 1);
				*volModUpdate = localParm->conn->dh->volMod;
				volMod = localParm->conn->dh->volMod;
				delayed = 0;
			}
			if (localParm->conn->audioDataFrame != NULL)
			{
				SA_DataPutOrderDataFrame((char*)localParm->conn->audioDataFrame, localParm->conn->dh->sessionPacket, localParm->conn->dh);
				size_t sizeSend = localParm->conn->runCode == 3 ? SA_GetEncriptySize(localParm->dataSize) : localParm->dataSize;
				char* sendData = (char*)localParm->conn->audioDataFrame;
				if(isCrypt) {
					unsigned char* sendEncrypt = malloc(sizeSend);
					if(sendEncrypt == NULL) {
						SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
						break;
					}
					SA_EncryptData((unsigned char*)localParm->conn->audioDataFrame, (int)localParm->dataSize, sendEncrypt, localParm->encryptCtx);
					sendData = (char*)sendEncrypt;
				}
				if (send(localParm->ctx->clientSocket, sendData, (int)sizeSend, 0) == SOCKET_ERROR)
				{
					if (tolerance > 5)
					{
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_DEBUG);
					}
					tolerance++;
					SA_Sleep(1000);
					break;
				}
				else
				{
					if (localParm->conn->data[DATASIZE + 2] != 0x00) 
					{
						localParm->conn->data[DATASIZE + 2] = 0x00;
						SA_Log("Msg send", LOG_NET, LOG_CLASS_DEBUG);
					}
					else 
					{
						lastPacket = time(NULL);
						localParm->conn->dh->sessionPacket++;
					}
					tolerance = 0;
				}
				localParm->conn->audioDataFrame != NULL ? free(localParm->conn->audioDataFrame) : (void)0;
				localParm->conn->audioDataFrame = NULL;
				if (localParm->conn->runCode == 3) {
					free(sendData);
				}
			}
			if (delayed) {
				SA_Sleep(localParm->delay);
			}
			if (exitSender) {
				break;
			}
		}
		SA_ThreadClose(revcThread);
		shutdown(localParm->ctx->clientSocket, SD_BOTH);
		closesocket(localParm->ctx->clientSocket);
		if (isCrypt) {
			SA_CloseCtx(localParm->decryptCtx);
			SA_CloseCtx(localParm->encryptCtx);
		}
		localParm->conn->dh->header = NULLHEADER;
	}
	closesocket(localParm->ctx->srvSocket);
	SA_WinNetEnd(parms);
}

static unsigned short int SA_NetSetupClient(connectParam* parms)
{
	const char *ivVoid = calloc(1, 128);
	parms->conn->key[0] != 0 ? SA_GenIV(parms->conn->dh->iv) : (void)0;
	parms->ctx->srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (parms->ctx->srvSocket == INVALID_SOCKET)
	{
		SA_Log("Socket failed!", LOG_NET, LOG_CLASS_ERROR);
	}
	else
	{
		SA_Log("Socket ok", LOG_NET, LOG_CLASS_INFO);
		if (parms->conn->head != NULL)
		{
			if (parms->conn->head->next == NULL)
			{
				free(parms->conn->head->data);
				free(parms->conn->head);
			}
			else
			{
				for (audioBuffer* i = parms->conn->head; i != NULL; i = i->next)
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
			parms->conn->head = NULL;
		}
		SA_NetResolveHost(parms, AF_INET);
		SA_Log("Connecting...", LOG_NET, LOG_CLASS_INFO);
		if (connect(parms->ctx->srvSocket, (SOCKADDR*)&parms->ctx->srvAddr, sizeof(parms->ctx->srvAddr)) != 0)
		{
			SA_Log("Connect failed!", LOG_NET, LOG_CLASS_WARNING);
			return -1;
		}
		else
		{
			SA_Log("Host connected, sending 'hello' code (1/3)", LOG_NET, LOG_CLASS_INFO);
			if (send(parms->ctx->srvSocket, (char*)parms->conn->dh, sizeof(*parms->conn->dh), 0) == SOCKET_ERROR)
			{
				SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
				return -1;
			}
			else
			{
				char * recvData = (char*)parms->conn->dh;
				size_t sizeRevc = sizeof(dataHandshake);
				if(parms->conn->key[0] != 0) {
					parms->decryptCtx = SA_IniCtx();
					SA_PrepareDecryptCtx(parms->decryptCtx, parms->conn->key, parms->conn->dh->iv);
					sizeRevc = SA_GetEncriptySize(sizeRevc);
					recvData = malloc(sizeRevc);
					if(recvData == NULL) {
						SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
					}
				}
				SA_Log("Hello send... Wait Handshake (2/3)", LOG_NET, LOG_CLASS_INFO);
				if (recv(parms->ctx->srvSocket, recvData, (int)sizeRevc, 0) == SOCKET_ERROR)
				{
					SA_Log("Recv failed!", LOG_NET, LOG_CLASS_WARNING);
					return -1;
				}
				else
				{
					if (strcmp((const char *)((dataHandshake*)recvData)->iv, ivVoid) == 0) {
						SA_CloseCtx(parms->decryptCtx);
						parms->decryptCtx = NULL;
					}
					else if(parms->decryptCtx != NULL) {
						unsigned char* recvDecrypt = malloc(sizeRevc);
						if(recvDecrypt == NULL) {
							SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
						}
						int sizeSend = SA_DecryptData((unsigned char*)recvData, (int)sizeRevc, recvDecrypt, parms->decryptCtx);
						if (sizeSend == 0) {
							SA_Log("Handshake failled, Wrong Password", LOG_NET, LOG_CLASS_WARNING);
							return 0;
						}
						memcpy_s(parms->conn->dh, sizeof(dataHandshake), recvDecrypt, sizeof(dataHandshake));
					}
					else if (strcmp((const char *)((dataHandshake*)recvData)->iv, ivVoid) != 0){
						SA_Log("Handshake failled, Wrong Password", LOG_NET, LOG_CLASS_WARNING);
						return 0;
					}
					if (send(parms->ctx->srvSocket, (char*)confirmConn, 2, 0) == SOCKET_ERROR)
					{
						SA_Log("Send failed!", LOG_NET, LOG_CLASS_WARNING);
						return -1;
					}
					else
					{
						SA_Log("Handshake completed (3/3)", LOG_NET, LOG_CLASS_INFO);
						parms->dataSize = SA_DataGetDataFrameSize(parms->conn->dh);
						SA_Log("Audio connection established", LOG_NET, LOG_CLASS_INFO);
						if (parms->decryptCtx != NULL)
						{
							parms->conn->runCode = 3;
						}
						else
						{
							parms->conn->runCode = 2;
						}
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
	parms->conn->data[DATASIZE + 2] = 0x00;
	size_t sizeData = DATASIZE + 2;
	if(parms->conn->runCode == 3) {
		sizeData = SA_GetEncriptySize(sizeData);
		parms->encryptCtx = SA_IniCtx();
		SA_PrepareEncryptCtx(parms->encryptCtx, parms->conn->key, parms->conn->dh->iv);
	}
	size_t count = 0;
	while (parms->conn->runCode != -1) {
		if (parms->conn->data[DATASIZE + 2] != 0x00) {
			char* sendMsg = parms->conn->data;
			if (parms->conn->runCode == 3) {
				unsigned char* sendEncrypt = malloc(sizeData);
				if (sendEncrypt == NULL) {
					SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
				}
				SA_EncryptData((unsigned char*)parms->conn->data, DATASIZE + 2, sendEncrypt, parms->encryptCtx);
				sendMsg = (char*)sendEncrypt;
			}
			if (send(parms->ctx->srvSocket, sendMsg, (int)sizeData, 0) == SOCKET_ERROR)
			{
				SA_Log("Msg failed!", LOG_NET, LOG_CLASS_DEBUG);
				SA_Sleep(100);
				if (count > 10) {
					SA_Log("Msg failed!", LOG_NET, LOG_CLASS_WARNING);
				}
				count++;
			}
			else
			{
				parms->conn->data[DATASIZE + 2] = 0x00;
				SA_Log("Msg send!", LOG_NET, LOG_CLASS_DEBUG);
				if (parms->conn->runCode == 3) {
					free(sendMsg);
				}
			}
		}
		SA_Sleep(1);
	}
}

static void SA_NetClient(void* parms)
{
	PaStream* stream = NULL;
	connectParam* localParm = (connectParam*)parms;
	size_t chunckWaveSize = 0;
	size_t err = 0;
	char* localData = NULL;
	char* msgStream = NULL;
	size_t rounds = 0;

	SA_WinNetOpen();
	while (localParm->conn->runCode != -1)
	{
		(localParm->conn->runCode) = 1;
		while (localParm->conn->runCode != -1) {
			int ret = SA_NetSetupClient(localParm);
			if (ret == 0) {
				localParm->conn->runCode = -1;
				closesocket(localParm->ctx->srvSocket);
			}else if(ret == 1)
			{
				break;
			}
		}
		if(localParm->conn->runCode == -1)
		{
			break;
		}
		size_t revcSize = localParm->dataSize;
		localParm->conn->runCode == 3 ? revcSize = SA_GetEncriptySize(revcSize) : 0;
		void* sendThread = SA_ThreadCreate(SA_NetClientSend, parms);
		localData = malloc(revcSize);
		if (localData)
		{
			stream = SA_AudioOpenStream(localParm->conn->device, 0, (void*)localParm->conn);
			SA_AudioStartStream(stream);
			int breakRevcLoop = 0;
			while (localParm->conn->runCode != -1 && !breakRevcLoop)
			{
				if (recv(localParm->ctx->srvSocket, (char*)localData, (int)revcSize, MSG_WAITALL) == SOCKET_ERROR)
				{
					err++;
					if (err > 10)
					{
						SA_Log("Close connection", LOG_NET, LOG_CLASS_WARNING);
						localParm->conn->runCode = -1;
					}
				}
				else
				{
					if (localParm->conn->runCode == 3) {
						char * unEncrypt = malloc(revcSize);
						if(unEncrypt == NULL) {
							SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
						}
						else {
							SA_DecryptData((unsigned char*)localData, (int)revcSize, (unsigned char*)unEncrypt, localParm->decryptCtx);
							memcpy_s(localData, localParm->dataSize, unEncrypt, localParm->dataSize);
							free(unEncrypt);
						}
					}
					err = 0;
					localParm->conn->dh->totalPacketSrv = SA_DataGetOrderDataFrame(localData, localParm->conn->dh);
					dataHeader* header = (dataHeader*)localData;
					float* volMod = NULL; 
					switch (header[0])
					{
					case DATA:
						chunckWaveSize = SA_DataGetWaveSize(localData);
						if (chunckWaveSize != localParm->conn->dh->waveSize * (size_t)localParm->conn->dh->channel)
						{
							SA_Log("Invalid data size", LOG_NET, LOG_CLASS_WARNING);
							break;
						}
						localParm->conn->audioDataFrame = malloc(localParm->dataSize);
						localParm->conn->audioDataFrame == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
						if (localParm->conn->audioDataFrame != NULL)
						{
							memcpy_s(localParm->conn->audioDataFrame, localParm->dataSize, localData, localParm->dataSize);
							audioBuffer* temp = (audioBuffer*)malloc(sizeof(audioBuffer));
							if (temp)
							{
								if (localParm->conn->wavFile != NULL)
								{
									SA_WavWriteData(localParm->conn->wavFile, SA_DataGetWaveData(localData), localParm->conn->dh->waveSize * localParm->conn->dh->channel,localParm->conn->rounds);
									localParm->conn->rounds++;
								}
								temp->data = localParm->conn->audioDataFrame;
								temp->next = NULL;
								temp->prev = NULL;
								if (localParm->conn->head == NULL)
								{
									localParm->conn->head = temp;
								}
								else
								{
									int bufferSize = 0;
									int delete = 0;
									for (audioBuffer* i = localParm->conn->head; i != NULL; i = i->next)
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
											localParm->conn->dh->sessionPacket--;
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
						localParm->conn->dh->sessionPacket++;
						break;
					case DATAMSG:
						SA_DataRevcProcess(&rounds, &msgStream, (char*)(header + 1), &localParm->conn->msg);
						SA_Log("Msg revc!", LOG_NET, LOG_CLASS_DEBUG);
						break;
					case END:
						SA_Log("Connection closed", LOG_NET, LOG_CLASS_INFO);
						breakRevcLoop = 1;
						break;
					case VOLUPDATE:
						SA_Log("Vol update", LOG_NET, LOG_CLASS_DEBUG);
						volMod = (float*)(((dataHeader*)localData) + 1);
						localParm->conn->dh->volMod = *volMod;
					case NULLDATA:
						break;
					default:
						SA_Log("Invalid header", LOG_NET, LOG_CLASS_WARNING);
						break;
					}
				}
			}
			SA_AudioCloseStream(stream);
			shutdown(localParm->ctx->srvSocket, SD_BOTH);
			closesocket(localParm->ctx->srvSocket);
			SA_ThreadClose(sendThread);
			SA_CloseCtx(localParm->decryptCtx);
			SA_CloseCtx(localParm->encryptCtx);
			free(localData);
		}
		else
		{
			SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
		}
	}
	SA_WinNetEnd(parms);
}

void SA_NetInit(saConnection * conn)
{
	connectParam* dataParm = NULL;
	dataParm = calloc(1,sizeof(connectParam));
	if (dataParm != NULL)
	{
		dataParm->ctx = calloc(1,sizeof(netCtx));
		dataParm->ctx == NULL ? SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR) : (void)0;
		dataParm->conn = conn;
		if (conn->mode)
		{
			conn->thread = SA_ThreadCreate(SA_NetClient, (void*)dataParm);
		}
		else
		{
			SA_NetSetupServer(dataParm);
			conn->thread = SA_ThreadCreate(SA_NetServer, (void*)dataParm);
		}
		conn->thread == NULL ? SA_Log("Failed to create thread", LOG_NET, LOG_CLASS_ERROR) : (void)0;
	}
	else
	{
		SA_Log("Failed to allocate memory", LOG_NET, LOG_CLASS_ERROR);
	}
}

void SA_NetClose(void* thread, saConnection* conn)
{
	if (conn->runCode == 1 && conn->mode == 0) {
		SA_ThreadClose(thread);
		return;
	}
	conn->runCode = -1;
	SA_ThreadJoin(thread);
}
