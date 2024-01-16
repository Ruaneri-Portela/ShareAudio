#include "data.h"
#include "log.h"
#include "portaudio/include/portaudio.h"
#include <stdio.h>
#include <string.h>

typedef struct audioBuffer
{
	void* data;
	struct audioBuffer* next;
	struct audioBuffer* prev;
} audioBuffer;

char* audioDataFrame = NULL;

audioBuffer* head = NULL;

unsigned short int testMode = 0;

unsigned short int barMode = 0;

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
	printf("\r");
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
	dataHandshake* lDh = (dataHandshake*)userData;
	drawBar((float*)outputBuffer, framesPerBuffer);
	if (head != NULL)
	{
		audioBuffer* temp = head;
		float* data = getWaveFrame(temp->data);
		copyInto(data, (float*)outputBuffer, framesPerBuffer * lDh->channel, volMod, testMode);
		head = temp->next;
		free(temp->data);
		free(temp);
		if (head != NULL && head->prev != NULL) {
			head->prev = NULL;
		}
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
	dataHandshake* lDh = (dataHandshake*)userData;
	drawBar((float*)inputBuffer, framesPerBuffer);
	if (audioDataFrame != NULL) {
		free(audioDataFrame);
	}
	audioDataFrame = createDataFrame((float*)inputBuffer, lDh, testMode);
	return 0;
}

void stopStream(PaStream* stream)
{
	checkErr(Pa_StopStream(stream));
	logCat("Stream stopped", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void startStream(PaStream* stream)
{
	checkErr(Pa_StartStream(stream));
	logCat("Stream started", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void initAudio()
{
	checkErr(Pa_Initialize());
	logCat("PortAudio initialized", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void closeAudio()
{
	checkErr(Pa_Terminate());
	logCat("PortAudio terminated", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void listAudioDevices()
{
	int numDevices = Pa_GetDeviceCount();
	printf("==============================\nDevices atrach on computer\nNumber of devices: %d\nListing avaliable devices...\n\n", numDevices);
	if (numDevices < 0)
	{
		checkErr(numDevices);
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
	PaStream* stream;
	char* msg = concatString("Using device: ", Pa_GetDeviceInfo(device)->name);
	logCat(msg, LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
	free(msg);
	asServer ? logCat("Server mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod) : logCat("Client mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
	if (asServer)
	{
		checkErr(Pa_OpenStream(
			&stream,
			&parms,
			NULL,
			sampleRate,
			waveSize,
			paNoFlag,
			serverCallback,
			dh));
	}
	else
	{
		checkErr(Pa_OpenStream(
			&stream,
			NULL,
			&parms,
			sampleRate,
			waveSize,
			paNoFlag,
			clientCallback,
			dh));
	}
	return stream;
}

void shutdownStream(PaStream* stream)
{
	checkErr(Pa_StopStream(stream));
	checkErr(Pa_CloseStream(stream));
	logCat("Stream shutdowner", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}