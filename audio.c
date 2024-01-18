#include <stdio.h>
#include <string.h>
#include "portaudio/include/portaudio.h"
#include "data.h"
#include "log.h"

typedef struct audioBuffer
{
	void* data;
	struct audioBuffer* next;
	struct audioBuffer* prev;
} audioBuffer;

char* audioDataFrame = NULL;

audioBuffer* head = NULL;

unsigned short int testMode = 0;

static inline float SA_AudioGetMax(float a, float b)
{
	return a > b ? a : b;
}

static inline float SA_AudioGetAbs(float a)
{
	return a > 0 ? a : a * -1;
}

static inline void SA_AudioCheckError(PaError err)
{
	if (err != paNoError)
	{
		SA_Log(Pa_GetErrorText(err), LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
	}
}

int SA_AudioClientCallback(
	const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
	void* userData)
{
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	if (head != NULL)
	{
		audioBuffer* temp = head;
		float* data = SA_DataGetWaveData(temp->data);
		SA_DataCopyAudio(data, (float*)outputBuffer, framesPerBuffer * ((dataHandshake*)userData)->channel, ((dataHandshake*)userData)->volMod, testMode);
		head = temp->next;
		free(temp->data);
		free(temp);
		if (head != NULL && head->prev != NULL) {
			head->prev = NULL;
		}
	}
	return 0;
}

int SA_AudioServerCallback(
	const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
	void* userData)
{
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)framesPerBuffer;
	if (audioDataFrame != NULL) {
		free(audioDataFrame);
	}
	audioDataFrame = SA_DataCreateDataFrame((float*)inputBuffer, userData, testMode);
	return 0;
}

void SA_AudioStopStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_StopStream(stream));
	SA_Log("Stream stopped", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void SA_AudioStartStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_StartStream(stream));
	SA_Log("Stream started", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void SA_AudioInit()
{
	SA_AudioCheckError(Pa_Initialize());
	SA_Log("PortAudio initialized", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void SA_AudioClose()
{
	SA_AudioCheckError(Pa_Terminate());
	SA_Log("PortAudio terminated", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void SA_AudioListAllDevices()
{
	int numDevices = Pa_GetDeviceCount();
	printf("==============================\nDevices atrach on computer\nNumber of devices: %d\nListing avaliable devices...\n\n", numDevices);
	if (numDevices < 0)
	{
		SA_AudioCheckError(numDevices);
	}
	else if (numDevices == 0)
	{
		SA_Log("No devices found", LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
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

PaStream* SA_AudioOpenStream(size_t device, size_t lchannel, double sampleRate, size_t waveSize, unsigned short asServer, void* configs)
{
	PaStreamParameters parms;
	memset(&parms, 0, sizeof(parms));
	parms.channelCount = (int)lchannel;
	parms.device = (int)device;
	parms.hostApiSpecificStreamInfo = NULL;
	parms.sampleFormat = paFloat32;
	parms.suggestedLatency = Pa_GetDeviceInfo((int)device)->defaultLowInputLatency;
	PaStream* stream;
	char* msg = SA_DataConcatString("Using device: ", Pa_GetDeviceInfo((int)device)->name);
	SA_Log(msg, LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
	free(msg);
	asServer ? SA_Log("Server mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod) : SA_Log("Client mode", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
	if (asServer)
	{
		SA_AudioCheckError(Pa_OpenStream(
			&stream,
			&parms,
			NULL,
			(int)sampleRate,
			(int)waveSize,
			paNoFlag,
			SA_AudioServerCallback,
			configs));
	}
	else
	{
		SA_AudioCheckError(Pa_OpenStream(
			&stream,
			NULL,
			&parms,
			(int)sampleRate,
			(int)waveSize,
			paNoFlag,
			SA_AudioClientCallback,
			configs));
	}
	return stream;
}

void SA_AudioCloseStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_CloseStream(stream));
	SA_Log("Stream shutdowner", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}