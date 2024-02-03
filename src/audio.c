#include <string.h>
#include <stdio.h>

#include "../portaudio/include/portaudio.h"
#include "data.h"
#include "log.h"

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#include "linux.h"
#endif

typedef struct audioDevices
{
	const PaDeviceInfo **devices;
	int numDevices;
} audioDevices;

typedef struct audioBuffer
{
	void *data;
	struct audioBuffer *next;
	struct audioBuffer *prev;
} audioBuffer;

char *audioDataFrame = NULL;

audioBuffer *head = NULL;

unsigned short int testMode = 0;

static void SA_AudioCheckError(PaError err)
{
	if (err != paNoError)
	{
		SA_Log(Pa_GetErrorText(err), LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
	}
}

int SA_AudioClientCallback(
	const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
	void *userData)
{
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	if (head != NULL)
	{
		audioBuffer *temp = head;
		float *data = SA_DataGetWaveData(temp->data);
		SA_DataCopyAudio(data, (float *)outputBuffer, framesPerBuffer * ((dataHandshake *)userData)->channel, ((dataHandshake *)userData)->volMod, testMode);
		head = temp->next;
		free(temp->data);
		free(temp);
		if (head != NULL)
		{
			head->prev = NULL;
		}
	}
	return 0;
}

int SA_AudioServerCallback(
	const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
	void *userData)
{
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)framesPerBuffer;
	if (audioDataFrame != NULL)
	{
		free(audioDataFrame);
		audioDataFrame = NULL;
	}
	audioDataFrame = SA_DataCreateDataFrame((float *)inputBuffer, userData, testMode);
	return 0;
}

void SA_AudioStopStream(PaStream *stream)
{
	SA_AudioCheckError(Pa_StopStream(stream));
	SA_Log("Stream stopped", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}

void SA_AudioStartStream(PaStream *stream)
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

audioDevices SA_GetAllDevices()
{
	int numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		SA_AudioCheckError(numDevices);
	}
	else if (numDevices == 0)
	{
		SA_Log("No devices found", LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
	}
	const PaDeviceInfo **devices = (const PaDeviceInfo **)malloc(sizeof(const PaDeviceInfo *) * (numDevices + 1));
	if (devices == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_AUDIO, LOG_CLASS_ERROR, logOutputMethod);
	}
	for (int i = 0; i < numDevices; i++)
	{
		devices[i] = Pa_GetDeviceInfo(i);
	}
	devices[numDevices] = NULL;
	audioDevices devicesData = {devices, numDevices};
	return devicesData;
}

void SA_AudioListAllDevices()
{
	audioDevices devicesData = SA_GetAllDevices();
	printf_s("Found %d devices\n\n", devicesData.numDevices);
	for (int i = 0;; i++)
	{
		if (devicesData.devices[i] != NULL)
		{
			printf_s("Device %d:\n\t%s\n\tSample Rate:%f\n", i,
					 devicesData.devices[i]->name,
					 devicesData.devices[i]->defaultSampleRate);
			if (devicesData.devices[i]->maxInputChannels > 0)
				printf_s("\tChannels Int:%d\n", devicesData.devices[i]->maxInputChannels);
			if (devicesData.devices[i]->maxOutputChannels > 0)
				printf_s("\tChannels Out:%d\n", devicesData.devices[i]->maxOutputChannels);
			printf_s("\n");
		}
		else
		{
			break;
		}
	}
	free(devicesData.devices);
}

PaStream *SA_AudioOpenStream(size_t device, size_t lchannel, double sampleRate, size_t waveSize, unsigned short asServer, void *configs)
{
	PaStreamParameters parms;
	memset(&parms, 0, sizeof(parms));
	parms.channelCount = (int)lchannel;
	parms.device = (int)device;
	parms.hostApiSpecificStreamInfo = NULL;
	parms.sampleFormat = paFloat32;
	parms.suggestedLatency = Pa_GetDeviceInfo((int)device)->defaultLowInputLatency;
	PaStream *stream;
	char *msg = SA_DataConcatString("Using device: ", Pa_GetDeviceInfo((int)device)->name);
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

void SA_AudioCloseStream(PaStream *stream)
{
	SA_AudioCheckError(Pa_CloseStream(stream));
	SA_Log("Stream shutdowner", LOG_AUDIO, LOG_CLASS_INFO, logOutputMethod);
}
