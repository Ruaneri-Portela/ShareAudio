#include "log.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum enumHeader
{
	NULLHEADER = 0x00,
	HANDSHAKE = 0x01,
	AUTH = 0x02,
	DATA = 0x03,
	NULLDATA = 0x04,
	AUTHCHANGE = 0xFD,
	DISCONNECT = 0xFE,
	END = 0xFF,
};

typedef enum enumHeader dataHeader;

typedef struct dataHandshake
{
	dataHeader header;
	size_t channel;
	double sampleRate;
	size_t waveSize;
} dataHandshake;

dataHandshake* dh = NULL;

void copyInto(float* in, float* out, size_t size, float volMod, size_t testMode) {
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
			out[i] = (float)((float)rand() / RAND_MAX * 2.0 - 1.0);
		}
		break;
	default:
		break;
	}

}

char* createDataFrame(const float* data, size_t waveSize)
{
	size_t memorySize = sizeof(dataHeader) + sizeof(size_t) + sizeof(float) * waveSize;
	char* dataFrame = (char*)malloc(memorySize);
	if (dataFrame != NULL) {
		memset(dataFrame, 0, memorySize);
	}
	dataHeader* header = (dataHeader*)dataFrame;
	if (header != NULL) {
		*header = DATA;
	}
	else {
		free(dataFrame);
		return NULL;
	}
	size_t* sizeWave = (size_t*)(header + 1);
	*sizeWave = waveSize;

	float* waveFrame = (float*)(sizeWave + 1);

	data == NULL ? copyInto(NULL, waveFrame, waveSize, 1, 1) : copyInto((float*)data, waveFrame, waveSize, 1, 0);
	return dataFrame;
}

float* getWaveFrame(const char* dataFrame)
{
	size_t detour = sizeof(dataHeader) + sizeof(size_t);
	return (float*)((char*)dataFrame + detour);
}

int getSampleSize(const char* dataFrame)
{
	size_t detour = sizeof(dataHeader);
	return *(int*)((char*)dataFrame + detour);
}

int getSize(dataHandshake* dhData) {
	return (int)(sizeof(dataHeader) + sizeof(size_t) + ((sizeof(float) * dhData->waveSize) * dhData->channel) * dhData->channel);
}

size_t getDelay(dataHandshake* dhData) {
	return (size_t)(((((float)dhData->waveSize * (float)dhData->channel) / (float)dhData->sampleRate) / 4.0) * 1000);;
}

int detectHost(const char* host, size_t asServer) {
	size_t hostSize = strlen(host);
	if (hostSize > 6 && hostSize < 16) {
		int dotCount = 0;
		int digitCount = 0;
		for (size_t i = 0; i <= hostSize; i++)
		{
			if (host[i] == '.' && digitCount < 4) {
				dotCount++;
				digitCount = 0;
			}
			isdigit(host[i]) ? digitCount++ : 0;
		};
		if (!asServer && (strcmp(host, "0.0.0.0") == 0)) {
			logCat("0.0.0.0 is forbidden on client", LOG_NET, LOG_CLASS_ERROR, logOutputMethod);
			return 0;
		}
		else if (dotCount == 3 && digitCount < 4) {
			logCat("Ipv4 address detected", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
			return 1;
		}
		else {
			logCat("Try resolve host", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
			return 2;
		}
	}
	logCat("Try resolve host", LOG_NET, LOG_CLASS_INFO, logOutputMethod);
	return 2;
}