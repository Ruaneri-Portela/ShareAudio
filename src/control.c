#include "audio.h"
#include "config.h"
#include "data.h"
#include "log.h"
#include "net.h"
#include <stdio.h>

typedef struct saConnection
{
	void* thread;
	void* audio;
} saConnection;

saConnection* SA_Server(int device, int port, const char* host)
{
	saConnection* conn = malloc(sizeof(saConnection));
	if (conn == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return NULL;
	}
	else {
		memset(conn, 0, sizeof(saConnection));
		SA_Log("Server", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
		if (dh->volMod == -1)
			dh->volMod = 1;
		const PaDeviceInfo* info = Pa_GetDeviceInfo(device);
		conn->audio = SA_AudioOpenStream(device, info->maxInputChannels, dh->sampleRate, dh->waveSize, 1, dh);
		SA_AudioStartStream(conn->audio);
		conn->thread = SA_NetInit(port, host, 0, device);
		if (conn->thread == NULL)
		{
			SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		}
	}
	return conn;
}

saConnection* SA_Client(int device, int port, const char* host)
{
	saConnection* conn = malloc(sizeof(saConnection));
	if (conn == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return NULL;
	}
	else {
		memset(conn, 0, sizeof(saConnection));
		SA_Log("Client", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
		conn->thread = SA_NetInit(port, host, 1, device);
		if (conn->thread == NULL)
		{
			SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
			return NULL;
		}
	}
	return conn;
}

void SA_Close(saConnection* conn)
{
	if (conn->audio)
		SA_AudioCloseStream(conn->audio);
	SA_NetClose(conn->thread);
	free(conn);
}

void SA_SetVolumeModifier(float vol)
{
	dh->volMod = vol;
}

float SA_GetVolumeModifier()
{
	return dh->volMod;
}

void SA_ListAllAudioDevices()
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
