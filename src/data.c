#include <portaudio.h>
#include <openssl/evp.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "log.h"
#include "wav.h"


typedef enum dataHeader
{
	NULLHEADER = 0x00,
	HANDSHAKE = 0x01,
	AUTH = 0x02,
	DATA = 0x03,
	NULLDATA = 0x04,
	DATAMSG = 0x05,
	AUTHCHANGE = 0xFD,
	DISCONNECT = 0xFE,
	END = 0xFF,
} dataHeader;

typedef struct audioDevices
{
	const PaDeviceInfo** devices;
	int numDevices;
} audioDevices;

typedef struct audioBuffer
{
	void* data;
	struct audioBuffer* next;
	struct audioBuffer* prev;
} audioBuffer;


typedef struct dataHandshake
{
	dataHeader header;
	int channel;
	double sampleRate;
	int waveSize;
	float volMod;
	int testMode;
	size_t sessionPacket;
	size_t totalPacketSrv;
	unsigned char iv[128];
} dataHandshake;

typedef struct saConnection
{
	char data[DATASIZE + 3];
	char* msg;

	FILE* wavFile;
	wavHeader* headerWav;
	int rounds;

	char* audioDataFrame;
	audioBuffer* head;

	dataHandshake* dh;
	const char* host;
	void* thread;
	void* audio;
	int device;
	int port;
	int mode;
	int runCode;
	unsigned char key[256];
} saConnection;

const unsigned char confirmConn[2] = { 0xFF, '\0' };

void SA_DataCopyAudio(float* in, float* out, size_t size, float volMod, size_t testMode)
{
	switch (testMode)
	{
	case 0:
		for (size_t i = 0; i < size; i++)
		{
			out[i] = in[i] * volMod;
		}
		break;
	case 1:
		for (size_t i = 0; i < size; i++)
		{
			out[i] = (float)((float)rand() / RAND_MAX * 2.0 - 1.0) * volMod;
		}
		break;
	default:
		break;
	}
}

size_t SA_DataGetDataFrameSize(dataHandshake* dhData)
{
	return (sizeof(*dhData) + (sizeof(size_t) * 2) + ((sizeof(float) * dhData->waveSize) * dhData->channel));
}

void SA_DataPutOrderDataFrame(char* dataFrame, size_t value, dataHandshake* dhData)
{
	size_t detour = SA_DataGetDataFrameSize(dhData) - sizeof(size_t);
	*(size_t*)((char*)dataFrame + detour) = value;
}

size_t SA_DataGetOrderDataFrame(char* dataFrame, dataHandshake* dhData)
{
	size_t detour = SA_DataGetDataFrameSize(dhData) - sizeof(size_t);
	return *(size_t*)((char*)dataFrame + detour);
}

char* SA_DataCreateDataFrame(const float* data, dataHandshake* dhData, unsigned short int testmode)
{
	int memorySize = SA_DataGetDataFrameSize(dhData);
	size_t audioPadding = dhData->waveSize * dhData->channel;
	char* dataFrame = (char*)malloc(memorySize);
	if (dataFrame != NULL)
	{
		memset(dataFrame, 0, memorySize);
		dataHeader* header = (dataHeader*)dataFrame;
		if (header != NULL)
		{
			*header = DATA;
		}
		else
		{
			free(dataFrame);
			return NULL;
		}
		size_t* sizeWave = (size_t*)(header + 1);
		*sizeWave = dhData->waveSize * dhData->channel;
		float* waveFrame = (float*)(sizeWave + 1);
		size_t* dataCount = (size_t*)(waveFrame + audioPadding);
		*dataCount = 0;
		testmode ? SA_DataCopyAudio(NULL, waveFrame, audioPadding, 1, 1) : SA_DataCopyAudio((float*)data, waveFrame, audioPadding, 1, 0);
		return dataFrame;
	}
	return NULL;
}

float* SA_DataGetWaveData(const char* dataFrame)
{
	size_t detour = sizeof(dataHeader) + (sizeof(size_t));
	return (float*)((char*)dataFrame + detour);
}

size_t SA_DataGetWaveSize(const char* dataFrame)
{
	size_t detour = sizeof(dataHeader);
	return *(size_t*)((char*)dataFrame + detour);
}

size_t SA_DataGetDelayInterFrames(dataHandshake* dhData)
{
	return (size_t)(((((float)dhData->waveSize * (float)dhData->channel) / (float)dhData->sampleRate) / 4.0) * 1000);
}

char* SA_DataConcatString(const char* original, const char* toCat)
{
	size_t originalSize = strlen(original);
	size_t toCatSize = strlen(toCat);
	char* newString = malloc(originalSize + toCatSize + 1);
	if (newString != NULL)
	{
		memcpy_s(newString, originalSize, original, originalSize);
		memcpy_s(newString + originalSize, toCatSize + 1, toCat, toCatSize + 1);
		return newString;
	}
	return NULL;
}

unsigned short int SA_DataDetectIsIp(const char* host, size_t asClient)
{
	size_t hostSize = strlen(host);
	if (hostSize > 6 && hostSize < 16)
	{
		int dotCount = 0;
		int digitCount = 0;
		int digit[3] = { -1, -1, -1 };
		for (size_t i = 0; i <= hostSize; i++)
		{
			if (isdigit(host[i]))
			{
				digit[digitCount] = (int)host[i] - 48;
			}
			if (host[i] == '.' && digitCount < 4)
			{
				dotCount++;
				digitCount = 0;
				int ipBlock = 0;
				if (digit[2] >= 0)
				{
					ipBlock = digit[0] * 100 + digit[1] * 10 + digit[2];
				}
				else if (digit[1] >= 0)
				{
					ipBlock = digit[0] * 10 + digit[1];
				}
				else
				{
					ipBlock = digit[0];
				}
				if (ipBlock > 255)
				{
					return 0;
				}
			}
			isdigit(host[i]) ? digitCount++ : 0;
		}
		if (asClient && (strcmp(host, "0.0.0.0") == 0))
		{
			return 2;
		}
		else if (dotCount == 3 && digitCount < 4)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void SA_DataCopyStr(char* target, const char* input, size_t sizeMax)
{
	size_t size = (sizeMax != 0) ? sizeMax : strlen(input);
	for (size_t i = 0; i <= size; i++) {
		target[i] = input[i];
	}
}

void SA_DataRevcProcess(size_t* rounds, char** msgStream, char* msgLocal, char** msg) {
	char* dataBuffer = malloc(DATASIZE * ((*rounds) + 1));
	if (*msgStream == NULL)
	{
		*msgStream = dataBuffer;
	}
	else
	{
		memcpy_s(dataBuffer, DATASIZE * (*rounds), *msgStream, DATASIZE * (*rounds));
		free(*msgStream);
		*msgStream = dataBuffer;
	}
	memcpy_s(*msgStream + (DATASIZE * (*rounds)), DATASIZE, msgLocal, DATASIZE);
	(*rounds)++;
	if (msgLocal[DATASIZE + 1] == 0x00) {
		*msg == NULL ? (void)0 : free(*msg);
		*msg = *msgStream;
		*msgStream = NULL;
		*rounds = 0;
	}
}