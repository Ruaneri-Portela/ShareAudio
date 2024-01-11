#include <stdlib.h>
#include <string.h>

enum enumHeader
{
	NULLHEADER = 0x00,
	HANDSHAKE = 0x01,
	AUTH = 0x02,
	DATA = 0x03,
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

void copyInto(float* in, float* out, size_t size, size_t volMod, size_t testMode) {
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

const int getSampleSize(const char* dataFrame)
{
	size_t detour = sizeof(dataHeader);
	return *(int*)((char*)dataFrame + detour);
}

dataHandshake getHandShake(char* dataFrame)
{
	dataHeader* header = (dataHeader*)dataFrame;
	size_t* sizeWave = (size_t*)(header + 1);
	float* waveFrame = (float*)(sizeWave + 1);
	dataHandshake data = { *header, *sizeWave, *waveFrame };
	return data;
}