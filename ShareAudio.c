#ifdef _MSC_VER
#define COMPILE "MSVC"
#elif __GNUC__
#define COMPILE "GCC"
#endif
#include "audio.h"
#include "data.h"
#include "log.h"
#include "winnet.h"
#include <stdio.h>

#define VERSION "0.0.1"

double sampleRate = 48000;
int framesPerBuffer = 2048;

char* host;
int port = 0;
int mode = 0;
int deviceAudio = 0;

void server(int device)
{
	logCat("Server", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	dh->channel = 2;
	dh->sampleRate = sampleRate;
	dh->waveSize = framesPerBuffer;
	const PaDeviceInfo* info = Pa_GetDeviceInfo(device);
	PaStream* stream = setupStream(device,info->maxInputChannels, dh->sampleRate, dh->waveSize, 1);
	startStream(stream);
	void* nThread = initNet(port, host, 0, device);
	if (nThread == NULL)
	{
		logCat("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return;
	}
	while (closeThread != NULL)
	{
		Sleep(1000);
	}
	shutdownStream(stream);
	closeNet(nThread);
}

void client(int device)
{
	logCat("Client", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	void* nThread = initNet(9950, host, 1, device);
	if (nThread == NULL)
	{
		logCat("Failed to init net", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return;
	}
	while (closeThread != NULL)
	{
		Sleep(1000);
	}
	closeNet(nThread);
}

int main(int argc, char* argv[])
{
	HANDLE hProcess = GetCurrentProcess();
	SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
	port = 9950;
	logOutputMethod = LOG_OUTPUT_CONSOLE;
	logCat("Program start, build on " COMPILE, LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
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
			else if (strcmp(argv[i], "-b") == 0)
			{
				barMode = 1;
			}
			else if (strcmp(argv[i], "-r") == 0)
			{
				initAudio();
				listAudioDevices();
				closeAudio();
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-v") == 0)
			{
				sscanf_s(argv[i + 1], "%f", &volMod);
				i++;
			}
			else if (strcmp(argv[i], "-h") == 0)
			{
				printf_s("ShareAudio\n\tA easy and light way to share your between your computers\n\tVer: %s\n", VERSION);
				printf_s("Usage: ShareAudio [options]\n");
				printf_s("Options:\n");
				printf_s("-a\t\tSet the ip address to connect to\n");
				printf_s("-b\t\tShow sond animated bar\n");
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
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-he") == 0)
			{
				printf_s("Examples:\n");
				printf_s("[1] ShareAudio -s -p 9950 -a 0.0.0.0 -d 0\n");
				printf_s("\tStart the program in server mode, listening on port 9950, on all interfaces, using audio device 0\n");
				printf_s("[2] ShareAudio -c - p 9950 -a 192.168.1.1 -d 0\n");
				printf_s("\tStart the program in client mode, connecting to 192.168.1.1 in port 9950, using audio device 0\n");
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-ht") == 0)
			{
				printf_s("Troubleshooting:\n");
				printf_s("[1] Allow this program on firewall in case connection is no prossible into computers\n");
				printf_s("[2] If 'Channel is not avalible' you as select non capture to server , and outup to client device\n\t Client accpets only out devices and server only capture or Loopbacks\n");
				printf_s("[3] If you have a problem with audio, try to change the audio device, some output on Windows not work propery\n\t Recommend select to outup to Windows 'Sound Mapper'\n");
				printf_s("[3] In bind error, change you port");
				return EXIT_SUCCESS;
			}
			else if (strcmp(argv[i], "-z") == 0)
			{
				sscanf_s(argv[i + 1], "%d", &framesPerBuffer);
				i++;
			}
			else if (strcmp(argv[i], "-x") == 0)
			{
				sscanf_s(argv[i + 1], "%lf", &sampleRate);
				i++;
			}
			else
			{
				printf_s("Unknown option: %s\n", argv[i]);
				return EXIT_SUCCESS;
			}
		}
	}
	else
	{
		logCat("Needs a comand line args", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		return EXIT_SUCCESS;
	}
	initAudio();
	dh = malloc(sizeof(dataHandshake));
	if (dh != NULL)
	{
		memset(dh, 0, sizeof(dataHandshake));
	}
	else
	{
		logCat("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
	}
	if (host == NULL)
	{
		host = malloc(sizeof(char) * 11);
		if (host != NULL) {
			strcpy_s(host, 11, "127.0.0.1\0");
		}
		else {
			logCat("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		}
	}
	switch (mode)
	{
	case 1:
		server(deviceAudio);
		break;
	case 2:
		client(deviceAudio);
		break;
	}
	free(dh);
	closeAudio();
	logCat("Program exit", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	return EXIT_SUCCESS;
}
