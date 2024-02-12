#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "wav.h"
#include "data.h"
#include "log.h"

static void SA_AudioCheckError(PaError err)
{
	if (err != paNoError)
	{
		SA_Log(Pa_GetErrorText(err), LOG_AUDIO, LOG_CLASS_ERROR);
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
	if (((saConnection*)userData)->head != NULL)
	{
		audioBuffer* temp = ((saConnection*)userData)->head;
		float* data = SA_DataGetWaveData(temp->data);

		SA_DataCopyAudio(data, (float*)outputBuffer, framesPerBuffer * ((saConnection*)userData)->dh->channel , ((saConnection*)userData)->dh->volMod, 0);
		((saConnection*)userData)->head = temp->next;
		free(temp->data);
		free(temp);
		if (((saConnection*)userData)->head != NULL)
		{
			((saConnection*)userData)->head->prev = NULL;
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
	if (((saConnection*)userData)->audioDataFrame == NULL)
	{
		if (((saConnection*)userData)->wavFile != NULL)
		{
			SA_WavWriteData(((saConnection*)userData)->wavFile, (float*)inputBuffer, framesPerBuffer * ((saConnection*)userData)->dh->channel, ((saConnection*)userData)->rounds);
			((saConnection*)userData)->rounds++;
		}
		((saConnection*)userData)->audioDataFrame = SA_DataCreateDataFrame((float*)inputBuffer, userData, ((dataHandshake*)userData)->testMode);
	}
	return 0;
}

void SA_AudioStopStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_StopStream(stream));
	SA_Log("Stream stopped", LOG_AUDIO, LOG_CLASS_INFO);
}

void SA_AudioStartStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_StartStream(stream));
	SA_Log("Stream started", LOG_AUDIO, LOG_CLASS_INFO);
}

EXPORT void SA_AudioInit()
{
	SA_AudioCheckError(Pa_Initialize());
	SA_Log("PortAudio initialized", LOG_AUDIO, LOG_CLASS_INFO);
}

EXPORT void SA_AudioClose()
{
	SA_AudioCheckError(Pa_Terminate());
	SA_Log("PortAudio terminated", LOG_AUDIO, LOG_CLASS_INFO);
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
		SA_Log("No devices found", LOG_AUDIO, LOG_CLASS_ERROR);
	}
	const PaDeviceInfo** devices = (const PaDeviceInfo**)malloc(sizeof(const PaDeviceInfo*) * (numDevices + 1));
	if (devices == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_AUDIO, LOG_CLASS_ERROR);
	}
	else
	{
		for (int i = 0; i < numDevices; i++)
		{
			devices[i] = Pa_GetDeviceInfo(i);
		}
		devices[numDevices] = NULL;
	}
	audioDevices devicesData = { devices, numDevices };
	return devicesData;
}

PaStream* SA_AudioOpenStream(size_t device, unsigned short asServer, saConnection* configs)
{
	PaStreamParameters parms;
	memset(&parms, 0, sizeof(parms));
	parms.channelCount = configs->dh->channel;
	parms.device = (int)device;
	parms.hostApiSpecificStreamInfo = NULL;
	parms.sampleFormat = paFloat32;
	parms.suggestedLatency = Pa_GetDeviceInfo((int)device)->defaultLowInputLatency;
	PaStream* stream;
	char* msg = SA_DataConcatString("Using device: ", Pa_GetDeviceInfo((int)device)->name);
	SA_Log(msg, LOG_AUDIO, LOG_CLASS_INFO);
	free(msg);
	asServer ? SA_Log("Server mode", LOG_AUDIO, LOG_CLASS_INFO) : SA_Log("Client mode", LOG_AUDIO, LOG_CLASS_INFO);
	if (asServer)
	{
		SA_AudioCheckError(Pa_OpenStream(
			&stream,
			&parms,
			NULL,
			configs->dh->sampleRate,
			configs->dh->waveSize,
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
			configs->dh->sampleRate,
			configs->dh->waveSize,
			paNoFlag,
			SA_AudioClientCallback,
			configs));
	}
	return stream;
}

void SA_AudioCloseStream(PaStream* stream)
{
	SA_AudioCheckError(Pa_CloseStream(stream));
	SA_Log("Stream shutdowner", LOG_AUDIO, LOG_CLASS_INFO);
}
