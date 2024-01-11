#include "audio.h"
#include "data.h"
#include "log.h"
#include "winnet.h"
#include <stdio.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 2048
#define VERSION "0.1.0 Experimental"

int device = 0;
char ip[32];
int port = 0;
int mode = 0;

void server(int device)
{
	logCat("Server", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	PaStream* srv = setupStream(device, 2, SAMPLE_RATE, FRAMES_PER_BUFFER, 1);
	void* nThread = initNet(port, ip, NULL, 0);
	startStream(srv);
	Sleep(100);
	while (runNet)
	{
		Sleep(1000);
	}
	shutdownStream(srv);
	closeNet(nThread);
}

void client(int device)
{
	logCat("Client", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	PaStream* cli = setupStream(device, 2, SAMPLE_RATE, FRAMES_PER_BUFFER, 0);
	void* nThread = initNet(9950, ip, NULL, 1);
	Sleep(500);
	while (runNet)
	{
		Sleep(1000);
	}
	shutdownStream(cli);
	closeNet(nThread);
}

int main(int argc, char* argv[])
{
	port = 9950;
	strcpy_s(ip, 32, "127.0.0.1");
	logOutputMethod = LOG_OUTPUT_CONSOLE;
	printf("ShareAudio\n\tA easy and light way to share your between your computers\n\tVer: %s\n", VERSION);
	logCat(Pa_GetVersionText(), LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
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
				device = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-a") == 0)
			{
				strcpy_s(ip, 32, argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-p") == 0)
			{
				port = atoi(argv[i + 1]);
				i++;
			}
			else if (strcmp(argv[i], "-t") == 0) {
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
				goto exit;
			}
			else if (strcmp(argv[i], "-v") == 0) {
				sscanf_s(argv[i + 1], "%f", &volMod);
				i++;
			}
			else
			{
				goto exit;
			}
		}
	}
	else
	{
		char choice;
		printf("Server 'S' or Client 'C': ");
		choice = getchar();
		if (choice == 'S' || choice == 's')
		{
			mode = 1;
			device = 2;
		}
		else if (choice == 'C' || choice == 'c')
		{
			mode = 2;
			device = 6;
		}
		else
		{
			logCat("Press s or c", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
		}
	}
	initAudio();
	listAudioDevices();
	dh = malloc(sizeof(dataHandshake));
	if(dh != NULL)
	{
		dh->header = HANDSHAKE;
		dh->channel = 2;
		dh->sampleRate = SAMPLE_RATE;
		dh->waveSize = FRAMES_PER_BUFFER;
	}
	else {
		logCat("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
	}
	switch (mode)
	{
	case 1:
		server(device);
		break;
	case 2:
		client(device);
		break;
	default:
		break;
	}
	free(dh);
exit:
	closeAudio();
	logCat("Program exit", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
	return EXIT_SUCCESS;
}
