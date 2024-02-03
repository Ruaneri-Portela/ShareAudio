#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#define COMPILE "MSVC"
#elif __GNUC__
#define COMPILE "GCC"
#endif

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#include "linux.h"
#define ISWIN 0
#else
#define ISWIN 1
#endif

#include "VERSION.h"
#include "threads.h"
#include "audio.h"
#include "data.h"
#include "log.h"
#include "net.h"

void server(int device, int port, const char* host);

void client(int device, int port, const char* host);

int main(int argc, char* argv[])
{
	unsigned short int mode = 0;
	const char* host = NULL;
	int deviceAudio = -1;
	int port = 9950;

	dh = malloc(sizeof(dataHandshake));
	if (dh != NULL)
	{
		dh->sampleRate = -1;
		dh->waveSize = 2048;
		dh->volMod = -1;
		dh->header = 0x0;
		dh->channel = 2;
	}
	else
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
	}

	SA_ProcessSetPriority();
	logOutputMethod = LOG_OUTPUT_CONSOLE;
	SA_Log("Program start. Build on " COMPILE ". Binary version " VERSION, LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	SA_Log(Pa_GetVersionText(), LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-s") == 0)
			{
				mode = 1;
			}
			else if (strcmp(argv[i], "-c") == 0)
			{
				mode = 2;
			}
			else if (strcmp(argv[i], "-d") == 0)
			{
				deviceAudio = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-a") == 0)
			{
				host = argv[i + 1];
				i++;
			}
			else if (strcmp(argv[i], "-p") == 0)
			{
				port = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-t") == 0)
			{
				testMode = 1;
			}
			else if (strcmp(argv[i], "-l") == 0)
			{
				logOutputMethod = LOG_OUTPUT_FILE;
			}
			else if (strcmp(argv[i], "-i") == 0)
			{
				printf_s("ShareAudio\n\tA easy and light way to share your between your computers\n\tVer: %s\n", VERSION);
			}
			else if (strcmp(argv[i], "-r") == 0)
			{
				SA_AudioInit();
				SA_AudioListAllDevices();
				SA_AudioClose();
				free(dh);
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-v") == 0)
			{
				sscanf_s(argv[i + 1], "%f", &dh->volMod);
				i++;
			}
			else if (strcmp(argv[i], "-h") == 0)
			{
				printf_s("ShareAudio\n\tA easy and light way to share your between your computers\n\tVer: %s\n", VERSION);
				printf_s("Usage: ShareAudio [options]\n");
				printf_s("Options:\n");
				printf_s("-a\t\tSet the ip address to connect to\n");
				printf_s("-c\t\tSet the program to client mode\n");
				printf_s("-d\t\tSet the audio device to use\n");
				printf_s("-h\t\tShow this help\n");
				printf_s("-l\t\tLog to file\n");
				printf_s("-p\t\tSet the port to connect to\n");
				printf_s("-r\t\tList audio devices\n");
				printf_s("-s\t\tSet the program to server mode\n");
				printf_s("-t\t\tTest mode\n");
				printf_s("-v\t\tSet the volume modifier\n");
				printf_s("-z\t\tSet the chunck size\n");
				printf_s("-x\t\tSet the sample rate\n\n");
				printf_s("-he\t\tTo view examples how use\n");
				printf_s("-ht\t\tTroubleshooting");
				printf_s("\n");
				free(dh);
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-he") == 0)
			{
				printf_s("Examples:\n");
				printf_s("[1] ShareAudio -s -p 9950 -a 0.0.0.0 -d 0\n");
				printf_s("\tStart the program in server mode, listening on port 9950, on all interfaces, using audio device 0\n");
				printf_s("[2] ShareAudio -c - p 9950 -a 192.168.1.1 -d 0\n");
				printf_s("\tStart the program in client mode, connecting to 192.168.1.1 in port 9950, using audio device 0\n");
				free(dh);
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-ht") == 0)
			{
				printf_s("Troubleshooting:\n");
				printf_s("[1] Allow this program on firewall in case connection is no prossible into computers\n");
				printf_s("[2] If 'Channel is not avalible' you as select non capture to server , and outup to client device\n\t Client accpets only out devices and server only capture or Loopbacks\n");
				printf_s("[3] If you have a problem with audio, try to change the audio device, some output on Windows not work propery\n\t Recommend select to outup to Windows 'Sound Mapper'\n");
				printf_s("[3] In bind error, change you port");
				free(dh);
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-z") == 0)
			{
				sscanf_s(argv[i + 1], "%zd", &(dh->waveSize));
				i++;
			}
			else if (strcmp(argv[i], "-x") == 0)
			{
				sscanf_s(argv[i + 1], "%lf", &(dh->sampleRate));
				i++;
			}
			else
			{
				printf_s("Unknown option: %s\n", argv[i]);
				free(dh);
				return EXIT_SUCCESS;
			}
		}
	}
	else
	{
		SA_Log("Needs a comand line args", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return EXIT_SUCCESS;
	}
	SA_AudioInit();

	if (deviceAudio == -1 && mode == 1 && ISWIN) {
		int defaultOutDevice = Pa_GetDefaultOutputDevice();
		const char* defaultOutDeviceName = Pa_GetDeviceInfo(defaultOutDevice)->name;
		audioDevices deviceList = SA_GetAllDevices();
		int loopbackDevice = -1;
		for (int i = 0;; i++) {
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
			deviceAudio = loopbackDevice;
		}
		else
		{
			SA_Log("Loopback device not found It's is cause by using old DLL or MSYS2 version...", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		}
	}
	else {
		(deviceAudio == -1 && mode == 2) ? deviceAudio = Pa_GetDefaultOutputDevice() : deviceAudio;
		(deviceAudio == -1 && mode == 1) ? deviceAudio = Pa_GetDefaultInputDevice() : deviceAudio;
	}
	(dh->sampleRate == -1 && deviceAudio != -1) ? dh->sampleRate = Pa_GetDeviceInfo(deviceAudio)->defaultSampleRate : dh->sampleRate;
	if (host == NULL)
	{
		host = "127.0.0.1";
	}
	switch (mode)
	{
	case 0:
		SA_Log("No mode selected", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		break;
	case 1:
		server(deviceAudio, port, host);
		break;
	case 2:
		client(deviceAudio, port, host);
		break;
	}
	SA_AudioClose();
	free(dh);
	SA_Log("Program exit", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	return EXIT_SUCCESS;
}

void server(int device, int port, const char* host)
{
	SA_Log("Server", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	if (dh->volMod == -1)
		dh->volMod = 1;
	const PaDeviceInfo* info = Pa_GetDeviceInfo(device);
	PaStream* stream = SA_AudioOpenStream(device, info->maxInputChannels, dh->sampleRate, dh->waveSize, 1, dh);
	SA_AudioStartStream(stream);
	void* nThread = SA_NetInit(port, host, 0, device);
	if (nThread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return;
	}
	while (closeThread != NULL)
		SA_Sleep(1000);
	SA_AudioCloseStream(stream);
	SA_NetClose(nThread);
}

void client(int device, int port, const char* host)
{
	SA_Log("Client", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	void* nThread = SA_NetInit(port, host, 1, device);
	if (nThread == NULL)
	{
		SA_Log("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return;
	}
	while (closeThread != NULL)
		SA_Sleep(1000);
	SA_NetClose(nThread);
}