#include "audio.h"
#include "config.h"
#include "data.h"
#include "log.h"
#include "net.h"
#include "threads.h"
#include <stdio.h>

EXPORT void SA_Init(saConnection* conn)
{
	SA_AudioInit();
	audioDevices deviceList = SA_GetAllDevices();
	if (deviceList.numDevices < conn->device)
	{
		SA_Log("Device out of range", LOG_MAIN, LOG_CLASS_ERROR);
	}
	if (conn->device == -1 && conn->mode == 1 && ISWIN)
	{
		int defaultOutDevice = Pa_GetDefaultOutputDevice();
		const char* defaultOutDeviceName = Pa_GetDeviceInfo(defaultOutDevice)->name;
		int loopbackDevice = -1;
		for (int i = 0;; i++)
		{
			if (deviceList.devices[i] != NULL)
			{
				const char* searchName = deviceList.devices[i]->name;
				if (strstr(searchName, defaultOutDeviceName) != NULL && strstr(searchName, "[Loopback]"))
				{
					loopbackDevice = i;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (loopbackDevice != -1)
		{
			conn->device = loopbackDevice;
		}
		else
		{
			SA_Log("Loopback device not found It's is cause by using old DLL or MSYS2 version...", LOG_MAIN, LOG_CLASS_ERROR);
		}
	}
	else
	{
		(conn->device == -1 && conn->mode == 2) ? conn->device = Pa_GetDefaultOutputDevice() : conn->device;
		(conn->device == -1 && conn->mode == 1) ? conn->device = Pa_GetDefaultInputDevice() : conn->device;
	}
	(conn->dh->sampleRate == -1 && conn->device > -1) ? conn->dh->sampleRate = Pa_GetDeviceInfo(conn->device)->defaultSampleRate : conn->dh->sampleRate;
	if (conn->host == NULL)
	{
		conn->host = "127.0.0.1";
	}
	conn->dh->volMod == -1 ? conn->dh->volMod = 1 : conn->dh->volMod;
}

EXPORT void SA_Server(saConnection* conn)
{
	SA_Log("Server", LOG_MAIN, LOG_CLASS_INFO);
	conn->audio = SA_AudioOpenStream(conn->device, 1, conn->dh);
	SA_AudioStartStream(conn->audio);
	conn->thread = SA_NetInit(conn->port, conn->host, 0, conn->device, conn->dh);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Client(saConnection* conn)
{
	SA_Log("Client", LOG_MAIN, LOG_CLASS_INFO);
	conn->thread = SA_NetInit(conn->port, conn->host, 1, conn->device, conn->dh);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Close(saConnection* conn)
{
	if (conn->audio)
		SA_AudioCloseStream(conn->audio);
	SA_NetClose(conn->thread);
	free(conn);
}

EXPORT void SA_SetVolumeModifier(float vol, saConnection* conn)
{
	conn->dh->volMod = vol;
}

EXPORT float SA_GetVolumeModifier(saConnection* conn)
{
	return conn->dh->volMod;
}

EXPORT void SA_ListAllAudioDevices()
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

EXPORT saConnection* SA_Setup(int device, const char* host, int mode, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate)
{
	saConnection* conn = malloc(sizeof(saConnection));
	if (conn == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
	}
	else
	{
		memset(conn, 0, sizeof(saConnection));
		conn->device = device;
		conn->host = host;
		conn->mode = mode;
		conn->port = port;
		conn->dh = malloc(sizeof(dataHandshake));
		if (conn->dh == NULL)
		{
			SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
		}
		else
		{
			memset(conn->dh, 0, sizeof(dataHandshake));
			conn->dh->testMode = testMode;
			conn->dh->sampleRate = sampleRate;
			conn->dh->waveSize = waveSize;
			conn->dh->volMod = volMod;
			conn->dh->header = 0x0;
			conn->dh->channel = channel;
			SA_ProcessSetPriority();
			logOutputMethod = LOG_OUTPUT_CONSOLE;
			SA_Log("Program start. Build on " COMPILE ". Binary version " VERSION, LOG_MAIN, LOG_CLASS_INFO);
			SA_Log(Pa_GetVersionText(), LOG_MAIN, LOG_CLASS_INFO);
		}
		SA_AudioInit();
	}
	return conn;
}

EXPORT void SA_Shutdown(saConnection* conn)
{
	SA_Close(conn);
	SA_AudioClose();
}

EXPORT const char* SA_GetStats(saConnection* conn)
{
	// UNSECURE CODE
	char stats[500];
	sprintf_s(stats, 500, "%zd,%zd,%d,%lf,%d,%s,%s,%d", conn->dh->totalPacketSrv, conn->dh->totalPacketSrv, conn->dh->channel,
		conn->dh->sampleRate, conn->dh->waveSize, Pa_GetDeviceInfo(conn->device)->name, conn->host, conn->port);
	return stats;
}

EXPORT int SA_TestDLL()
{
	SA_Log("OK DLL", LOG_MAIN, LOG_CLASS_INFO);
	return 1;
}

EXPORT void SA_SetLogNULL() {
	logOutputMethod = LOG_OUTPUT_NULL;
}

EXPORT void SA_SetLogFILE(const char* filename) {
	logOutputMethod = LOG_OUTPUT_FILE;
	fileLogName = filename;
}

EXPORT void SA_SetLogCONSOLE()
{
	logOutputMethod = LOG_OUTPUT_CONSOLE;
}