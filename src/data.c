#include "config.h"
#include "log.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef enum dataHeader
{
	NULLHEADER = 0x00,
	HANDSHAKE = 0x01,
	AUTH = 0x02,
	DATA = 0x03,
	NULLDATA = 0x04,
	AUTHCHANGE = 0xFD,
	DISCONNECT = 0xFE,
	END = 0xFF,
} dataHeader;

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
} dataHandshake;

typedef struct saConnection
{
	void *thread;
	void *audio;
	int port;
	int device;
	const char *host;
	int mode;
	dataHandshake *dh;
} saConnection;

void SA_DataCopyAudio(float *in, float *out, size_t size, float volMod, size_t testMode)
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

size_t SA_DataGetDataFrameSize(dataHandshake *dhData)
{
	return (sizeof(*dhData) + (sizeof(size_t) * 2) + ((sizeof(float) * dhData->waveSize) * dhData->channel));
}

void SA_DataPutOrderDataFrame(char *dataFrame, size_t value, dataHandshake *dhData)
{
	size_t detour = SA_DataGetDataFrameSize(dhData) - sizeof(size_t);
	*(size_t *)((char *)dataFrame + detour) = value;
}

size_t SA_DataGetOrderDataFrame(char *dataFrame, dataHandshake *dhData)
{
	size_t detour = SA_DataGetDataFrameSize(dhData) - sizeof(size_t);
	return *(size_t *)((char *)dataFrame + detour);
}

char *SA_DataCreateDataFrame(const float *data, dataHandshake *dhData, unsigned short int testmode)
{
	size_t memorySize = SA_DataGetDataFrameSize(dhData);
	size_t audioPadding = dhData->waveSize * dhData->channel;
	char *dataFrame = (char *)malloc(memorySize);
	if (dataFrame != NULL)
	{
		memset(dataFrame, 0, memorySize);
	}
	dataHeader *header = (dataHeader *)dataFrame;
	if (header != NULL)
	{
		*header = DATA;
	}
	else
	{
		free(dataFrame);
		return NULL;
	}
	size_t *sizeWave = (size_t *)(header + 1);
	*sizeWave = dhData->waveSize * dhData->channel;
	float *waveFrame = (float *)(sizeWave + 1);
	size_t *dataCount = (size_t *)(waveFrame + audioPadding);
	*dataCount = 0;
	testmode ? SA_DataCopyAudio(NULL, waveFrame, audioPadding, 1, 1) : SA_DataCopyAudio((float *)data, waveFrame, audioPadding, 1, 0);
	return dataFrame;
}

float *SA_DataGetWaveData(const char *dataFrame)
{
	size_t detour = sizeof(dataHeader) + (sizeof(size_t));
	return (float *)((char *)dataFrame + detour);
}

size_t SA_DataGetWaveSize(const char *dataFrame)
{
	size_t detour = sizeof(dataHeader);
	return *(size_t *)((char *)dataFrame + detour);
}

size_t SA_DataGetDelayInterFrames(dataHandshake *dhData)
{
	return (size_t)(((((float)dhData->waveSize * (float)dhData->channel) / (float)dhData->sampleRate) / 4.0) * 1000);
}

char *SA_DataConcatString(const char *original, const char *toCat)
{
	size_t originalSize = strlen(original);
	size_t toCatSize = strlen(toCat);
	char *newString = malloc(originalSize + toCatSize + 1);
	if (newString != NULL)
	{
		memcpy_s(newString, originalSize, original, originalSize);
		memcpy_s(newString + originalSize, toCatSize + 1, toCat, toCatSize + 1);
	}
	return newString;
}

unsigned short int SA_DataDetectIsIp(const char *host, size_t asServer)
{
	size_t hostSize = strlen(host);
	if (hostSize > 6 && hostSize < 16)
	{
		int dotCount = 0;
		int digitCount = 0;
		int digit[3] = {-1, -1, -1};
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
		if (!asServer && (strcmp(host, "0.0.0.0") == 0))
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