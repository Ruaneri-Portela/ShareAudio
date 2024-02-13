#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <portaudio.h>
#include <openssl/evp.h>

#include "config.h"
#include "wav.h"
#include "data.h"
#include "audio.h"
#include "log.h"
#include "net.h"
#include "threads.h"
#include "encrypt.h"

EXPORT void SA_Init(saConnection* conn)
{
	SA_AudioInit();
	audioDevices deviceList = SA_GetAllDevices();
	if (deviceList.numDevices < conn->device)
	{
		SA_Log("Device out of range", LOG_MAIN, LOG_CLASS_ERROR);
	}
	if (conn->device == -1 && conn->mode == 0 && ISWIN)
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
		(conn->device == -1 && conn->mode == 1) ? conn->device = Pa_GetDefaultOutputDevice() : conn->device;
		(conn->device == -1 && conn->mode == 0) ? conn->device = Pa_GetDefaultInputDevice() : conn->device;
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
	conn->mode = 0;
	conn->audio = SA_AudioOpenStream(conn->device, 1, conn);
	SA_AudioStartStream(conn->audio);
	SA_NetInit(conn);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Client(saConnection* conn)
{
	SA_Log("Client", LOG_MAIN, LOG_CLASS_INFO);
	conn->mode = 1;
	SA_NetInit(conn);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Close(saConnection* conn)
{
	if (conn != NULL)
	{
		conn->audio  != NULL ? SA_AudioCloseStream(conn->audio), conn->audio = NULL : 0;
		conn->thread != NULL ? SA_NetClose(conn->thread, conn), conn->thread = NULL : 0;
		free(conn);
	}
}

EXPORT void SA_SetVolumeModifier(float vol, saConnection* conn)
{
	conn->dh->volMod = vol;
}

EXPORT float SA_GetVolumeModifier(saConnection* conn)
{
	return conn->dh->volMod;
}

EXPORT void SA_ListAllAudioDevices(saConnection* conn)
{
	conn == NULL ? SA_AudioInit() : 0;
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
	conn == NULL ? SA_AudioClose() : 0;
}

EXPORT const char* SA_ListAllAudioDevicesStr(saConnection* conn)
{
	char* temp = malloc(DATASIZE);
	if (temp == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
	}
	else {
		temp[0] = '\0';
		char* tempLocal;
		char stringValue[20];
		conn == NULL ? SA_AudioInit() : 0;
		audioDevices devicesData = SA_GetAllDevices();
		for (int i = 0;; i++)
		{
			if (devicesData.devices[i] != NULL)
			{
				tempLocal = temp;
				temp = SA_DataConcatString(temp, devicesData.devices[i]->name);
				free(tempLocal);
				sprintf_s(stringValue, 20, ",%.0lf", devicesData.devices[i]->defaultSampleRate);
				tempLocal = temp;
				temp = SA_DataConcatString(temp, stringValue);
				sprintf_s(stringValue, 20, ",%d", devicesData.devices[i]->maxInputChannels);
				tempLocal = temp;
				temp = SA_DataConcatString(temp, stringValue);
				free(tempLocal);
				sprintf_s(stringValue, 20, ",%d,%d\n", devicesData.devices[i]->maxOutputChannels, i);
				tempLocal = temp;
				temp = SA_DataConcatString(temp, stringValue);
				free(tempLocal);
			}
			else
			{
				break;
			}
		}
		free(devicesData.devices);
		conn == NULL ? SA_AudioClose() : 0;
	}
	return temp;
}

EXPORT void SA_Free(void* data)
{
	free(data);
}

EXPORT const char* SA_Version()
{
	char* temp = SA_DataConcatString(VERSION, ",");
	char* temp2 = SA_DataConcatString(temp, COMPILE);
	free(temp);
	temp = SA_DataConcatString(temp2, ",");
	temp2 = SA_DataConcatString(temp, Pa_GetVersionText());
	free(temp);
	return temp2;
}

EXPORT saConnection* SA_Setup(int device, const char* host, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate)
{
	saConnection* conn = calloc(1, sizeof(saConnection));
	if (conn == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
	}
	else
	{
		conn->device = device;
		conn->host = host;
		conn->port = port;
		conn->dh = calloc(1,sizeof(dataHandshake));
		if (conn->dh == NULL)
		{
			SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
		}
		else
		{
			conn->dh->testMode = testMode;
			conn->dh->sampleRate = sampleRate;
			conn->dh->waveSize = waveSize;
			conn->dh->volMod = volMod;
			conn->dh->channel = channel;
			SA_ProcessSetPriority();
			SA_Log("Program start. Build on " COMPILE ". Binary version " VERSION, LOG_MAIN, LOG_CLASS_INFO);
			SA_Log(Pa_GetVersionText(), LOG_MAIN, LOG_CLASS_INFO);
		}
	}
	return conn;
}

EXPORT const char* SA_GetStats(saConnection* conn)
{
	char* stats = malloc(DATASIZE);
	if (stats == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
	}
	else {
		sprintf_s(stats, DATASIZE, "%zd,%zd,%d,%0.lf,%d,%s,%s,%d,%d", conn->dh->totalPacketSrv + 1, conn->dh->sessionPacket, conn->dh->channel,
			conn->dh->sampleRate, conn->dh->waveSize, Pa_GetDeviceInfo(conn->device)->name, conn->host, conn->port,conn->runCode);
	}
	return stats;
}

EXPORT void SA_SetLogNULL()
{
	logOutputMethod = LOG_OUTPUT_NULL;
}

EXPORT void SA_SetLogFILE(const char* filename, int debug)
{
	if (debug == 0)
	{
		logOutputMethod = LOG_OUTPUT_FILE;
	}
	else
	{
		logOutputMethod = LOG_OUTPUT_FILE_DEBUG;
		fileLogName = filename;
	}
}
EXPORT void SA_SetLogCONSOLE(int debug)
{
	if (debug == 0)
	{
		logOutputMethod = LOG_OUTPUT_CONSOLE;
	}
	else
	{
		logOutputMethod = LOG_OUTPUT_CONSOLE_DEBUG;
	}
}

EXPORT int SA_TestDLL()
{
	logOutput l = logOutputMethod;
	logOutputMethod = LOG_OUTPUT_CONSOLE;
	SA_SetLogCONSOLE(1);
	SA_Log("OK", LOG_MAIN, LOG_CLASS_DEBUG);
	logOutputMethod = l;
	return 1;
}

EXPORT const char* SA_ReadLastMsg(saConnection* conn)
{
	return conn->msg;
}

EXPORT int SA_SendMsg(const char* dataMsg, saConnection* conn)
{
	size_t size = strlen(dataMsg);
	size_t round = (size_t)ceil((double)size / DATASIZE);
	for (size_t i = 0; i < round; i++)
	{
		if (i == (round - 1))
		{
			SA_Log("Parsing Data Last", LOG_MAIN, LOG_CLASS_DEBUG);
			conn->data[DATASIZE + 1] = 0x00;
			SA_DataCopyStr(conn->data, dataMsg + (i * DATASIZE), NULL);
		}
		else
		{
			SA_Log("Parsing Data", LOG_MAIN, LOG_CLASS_DEBUG);
			conn->data[DATASIZE + 1] = 0x01; 
			SA_DataCopyStr(conn->data, dataMsg + (i * DATASIZE), DATASIZE);
		}
		conn->data[DATASIZE + 2] = 0x01;
		SA_Log("Wait sender on network", LOG_MAIN, LOG_CLASS_DEBUG);
		while (conn->data[DATASIZE + 2] != 0x00)
		{
			continue;
		}
		SA_Log("Packet has sender", LOG_MAIN, LOG_CLASS_DEBUG);
	}
	return 1;
}

EXPORT void SA_InitWavRecord(saConnection* conn, const char* path) {
	wavHeader* headData = SA_WavCreateHeader((int32_t)conn->dh->sampleRate, 32, conn->dh->channel, conn->dh->waveSize);
	conn->wavFile = SA_WavCreateFile(headData, path);
	free(headData);
}

EXPORT void SA_CloseWavRecord(saConnection* conn) {
	SA_WavCloseFile(conn->wavFile);
	conn->wavFile = NULL;
}

EXPORT void* SA_GetWavFilePtr(saConnection* conn) {
	return conn->wavFile;
}

EXPORT void SA_SetKey(saConnection* conn, const char *key)
{
		SA_SetupKey(key, conn->key);
}