#include "audio.h"
#include "config.h"
#include "data.h"
#include "log.h"
#include "net.h"
#include "threads.h"
#include "wav.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

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
	conn->thread = SA_NetInit(conn->port, conn->host, 0, conn->device, &conn->exit, conn->dh);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Client(saConnection* conn)
{
	SA_Log("Client", LOG_MAIN, LOG_CLASS_INFO);
	conn->thread = SA_NetInit(conn->port, conn->host, 1, conn->device, &conn->exit, conn->dh);
	if (conn->thread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR);
	}
}

EXPORT void SA_Close(saConnection* conn)
{
	if (conn != NULL)
	{
		if (conn->audio != NULL)
			SA_AudioCloseStream(conn->audio);
		if (*conn->thread != NULL)
			SA_NetClose(*conn->thread, conn);
		*conn->thread = NULL;
		conn->audio = NULL;
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
	if (conn == NULL)
	{
		SA_AudioInit();
	}
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
	if (conn == NULL)
	{
		SA_AudioClose();
	}
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
		if (conn == NULL)
		{
			SA_AudioInit();
		}
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
		if (conn == NULL)
		{
			SA_AudioClose();
		}
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
		conn->exit = 0;
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
			SA_Log("Program start. Build on " COMPILE ". Binary version " VERSION, LOG_MAIN, LOG_CLASS_INFO);
			SA_Log(Pa_GetVersionText(), LOG_MAIN, LOG_CLASS_INFO);
		}
	}
	return conn;
}

EXPORT const char* SA_GetStats(saConnection* conn)
{
	// UNSECURE CODE
	char* stats = malloc(DATASIZE);
	if (stats == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR);
	}
	else {
		sprintf_s(stats, DATASIZE, "%zd,%zd,%d,%0.lf,%d,%s,%s,%d,%d", conn->dh->totalPacketSrv + 1, conn->dh->sessionPacket, conn->dh->channel,
			conn->dh->sampleRate, conn->dh->waveSize, Pa_GetDeviceInfo(conn->device)->name, conn->host, conn->port,conn->exit);
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

EXPORT const char* SA_ReadLastMsg()
{
	return msg;
}

EXPORT int SA_SendMsg(const char* dataMsg)
{
	size_t size = strlen(dataMsg);
	size_t round = (size_t)ceil((double)size / DATASIZE);
	for (size_t i = 0; i < round; i++)
	{
		if (i == (round - 1))
		{
			SA_Log("Parsing Data Last", LOG_MAIN, LOG_CLASS_DEBUG);
			data[DATASIZE + 1] = 0x00;
		}
		else
		{
			SA_Log("Parsing Data", LOG_MAIN, LOG_CLASS_DEBUG);
			data[DATASIZE + 1] = 0x01;
		}
		SA_DataCopyStr(data, dataMsg + (i * DATASIZE));
		data[DATASIZE + 2] = 0x01;
		SA_Log("Wait sender on network", LOG_MAIN, LOG_CLASS_DEBUG);
		while (data[DATASIZE + 2] != 0x00)
		{
			continue;
		}
		SA_Log("Packet has sender", LOG_MAIN, LOG_CLASS_DEBUG);
	}
	return 1;
}

EXPORT void SA_InitWavRecord(saConnection* conn, const char* path) {
	wavHeader* headData = SA_WavCreateHeader(conn->dh->sampleRate, 32, conn->dh->channel, conn->dh->waveSize);
	wavFile = SA_WavCreateFile(headData, path);
	free(headData);
}

EXPORT void SA_CloseWavRecord() {
	SA_WavCloseFile(wavFile);
	wavFile = NULL;
}

EXPORT void* SA_GetWavFileP() {
	return wavFile;
}

EXPORT void SA_SetWavFileP(FILE* file) {
	wavFile = file;
}
