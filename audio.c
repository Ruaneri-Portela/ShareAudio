#include <stdio.h>
#include <string.h>
#include "data.h"
#include "log.h"
#include "portaudio/include/portaudio.h"

typedef struct audioBuffer
{
	char* data;
	void* next;
} audioBuffer;

void* audioDataFrame = NULL;

audioBuffer* head = NULL;

PaStream* globalStream = NULL;

int deviceAudio = 0;

static PaError err;

static size_t channel;

int testMode = 0;

int barMode = 0;

float volMod = 1;

static inline float maxLocal(float a, float b)
{
	return a > b ? a : b;
}

static inline float absLocal(float a)
{
	return a > 0 ? a : a * -1;
}

static inline void checkErr(PaError err)
{
	if (err != paNoError)
	{
		logCat(Pa_GetErrorText(err), LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
	}
}

static void drawBar(float* data, unsigned long framesPerBuffer)
{
	if (!barMode)
	{
		return;
	}
	int dispSize = 100;
	float volL = 0;
	float volR = 0;
	for (unsigned long i = 0; i < framesPerBuffer * 2; i += 2)
	{
		volL = maxLocal(volL, absLocal(data[i]));
		volR = maxLocal(volR, absLocal(data[i + 1]));
	}
	printf("\033[G");
	for (int i = 0; i < dispSize; i++)
	{
		float barProportion = i / (float)dispSize;
		if (barProportion <= volL && barProportion <= volR)
		{
			printf("█");
		}
		else if (barProportion <= volL)
		{
			printf("▀");
		}
		else if (barProportion <= volR)
		{
			printf("▄");
		}
		else
		{
			printf(" ");
		}
	}
	fflush(stdout);
}

int clientCallback(
	const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
	void* userData)
{
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;
	drawBar((float*)outputBuffer, framesPerBuffer);
	if (head != NULL)
	{
		audioBuffer* temp = head;
		float* data = getWaveFrame(temp->data);
		copyInto(data, (float*)outputBuffer, framesPerBuffer * channel, volMod, testMode);
		head = temp->next;
		free(temp->data);
		free(temp);
	}
	return 0;
}

int serverCallback(
	const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
	void* userData)
{
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;
	drawBar((float*)inputBuffer, framesPerBuffer);
	if (audioDataFrame != NULL) {
		free(audioDataFrame);
	}
	if (testMode)
	{
		audioDataFrame = createDataFrame(NULL, framesPerBuffer * channel);
	}
	else
	{
		audioDataFrame = createDataFrame((float*)inputBuffer, framesPerBuffer * channel);
	}
	return 0;
}

void stopStream(PaStream* s)
{
	err = Pa_StopStream(s);
	checkErr(err);
	logCat("Stream stopped", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void startStream(PaStream* s)
{
	err = Pa_StartStream(s);
	checkErr(err);
	logCat("Stream started", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void initAudio()
{
	err = Pa_Initialize();
	checkErr(err);
	logCat("PortAudio initialized", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void closeAudio()
{
	err = Pa_Terminate();
	checkErr(err);
	logCat("PortAudio terminated", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void listAudioDevices()
{
	int numDevices = Pa_GetDeviceCount();
	printf("==============================\nDevices atrach on computer\nNumber of devices: %d\nListing avaliable devices...\n\n", numDevices);
	if (numDevices < 0)
	{
		err = numDevices;
		checkErr(err);
	}
	else if (numDevices == 0)
	{
		logCat("No devices found", LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
		exit(EXIT_SUCCESS);
	}
	const PaDeviceInfo* deviceInfo;
	for (int i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("Device [%d]:\n\tname: %s\n\tmaxInputChannels: %d\n\tmaxOutputChannels: %d\n\tdefaultSampleRate: %f\n\n",
			i,
			deviceInfo->name,
			deviceInfo->maxInputChannels,
			deviceInfo->maxOutputChannels,
			deviceInfo->defaultSampleRate);
	}
	printf("==============================\n");
}

PaStream* setupStream(int device, int lchannel, double sampleRate, int waveSize, unsigned short asServer)
{
	PaStreamParameters parms;
	memset(&parms, 0, sizeof(parms));
	parms.channelCount = lchannel;
	parms.device = device;
	parms.hostApiSpecificStreamInfo = NULL;
	parms.sampleFormat = paFloat32;
	parms.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
	channel = lchannel;
	PaStream* stream;
	printf_s("Using device [%d]: %s\n",
		device,
		(Pa_GetDeviceInfo(device)->name));
	asServer ? logCat("Server mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod) : logCat("Client mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
	if (asServer)
	{
		err = Pa_OpenStream(
			&stream,
			&parms,
			NULL,
			sampleRate,
			waveSize,
			paNoFlag,
			serverCallback,
			NULL);
		checkErr(err);
	}
	else
	{
		err = Pa_OpenStream(
			&stream,
			NULL,
			&parms,
			sampleRate,
			waveSize,
			paNoFlag,
			clientCallback,
			NULL);
		checkErr(err);
	}
	globalStream = stream;
	if (barMode) {
		printf("\033[2J");
	}
	return stream;
}

void shutdownStream(PaStream* stream)
{
	err = Pa_StopStream(stream);
	checkErr(err);
	err = Pa_CloseStream(stream);
	checkErr(err);
	logCat("Stream shutdowner", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}