#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "config.h"
#include "data.h"
#include "control.h"
#include "log.h"
#include "net.h"
#include "threads.h"

int main(int argc, char *argv[])
{
	saConnection *conn = malloc(sizeof(saConnection));
	if (conn == NULL)
	{
		SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
	}
	else
	{
		memset(conn, 0, sizeof(saConnection));
		conn->device = -1;
		conn->host = NULL;
		conn->mode = 0;
		conn->port = 9950;
		conn->dh = malloc(sizeof(dataHandshake));
		memset(conn->dh, 0, sizeof(dataHandshake));
		conn->dh->testMode = 0;
		if (conn->dh == NULL)
		{
			SA_Log("Failed to allocate memory", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
		}
		else
		{
			conn->dh->sampleRate = -1;
			conn->dh->waveSize = 2048;
			conn->dh->volMod = -1;
			conn->dh->header = 0x0;
			conn->dh->channel = 2;
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
						conn->mode = 1;
					}
					else if (strcmp(argv[i], "-c") == 0)
					{
						conn->mode = 2;
					}
					else if (strcmp(argv[i], "-d") == 0)
					{
						conn->device = atoi(argv[i + 1]);
						i++;
					}
					else if (strcmp(argv[i], "-a") == 0)
					{
						conn->host = argv[i + 1];
						i++;
					}
					else if (strcmp(argv[i], "-p") == 0)
					{
						conn->port = atoi(argv[i + 1]);
						i++;
					}
					else if (strcmp(argv[i], "-t") == 0)
					{
						conn->dh->testMode = 1;
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
						SA_ListAllAudioDevices();
						SA_AudioClose();
						free(conn->dh);
						free(conn);
						return EXIT_SUCCESS;
					}
					else if (strcmp(argv[i], "-v") == 0)
					{
						sscanf_s(argv[i + 1], "%f", &(conn->dh->volMod));
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
						free(conn->dh);
						free(conn);
						return EXIT_SUCCESS;
					}
					else if (strcmp(argv[i], "-he") == 0)
					{
						printf_s("Examples:\n");
						printf_s("[1] ShareAudio -s -p 9950 -a 0.0.0.0 -d 0\n");
						printf_s("\tStart the program in server mode, listening on port 9950, on all interfaces, using audio device 0\n");
						printf_s("[2] ShareAudio -c - p 9950 -a 192.168.1.1 -d 0\n");
						printf_s("\tStart the program in client mode, connecting to 192.168.1.1 in port 9950, using audio device 0\n");
						free(conn->dh);
						free(conn);
						return EXIT_SUCCESS;
					}
					else if (strcmp(argv[i], "-ht") == 0)
					{
						printf_s("Troubleshooting:\n");
						printf_s("[1] Allow this program on firewall in case connection is no prossible into computers\n");
						printf_s("[2] If 'Channel is not avalible' you as select non capture to server , and outup to client device\n\t Client accpets only out devices and server only capture or Loopbacks\n");
						printf_s("[3] If you have a problem with audio, try to change the audio device, some output on Windows not work propery\n\t Recommend select to outup to Windows 'Sound Mapper'\n");
						printf_s("[3] In bind error, change you port");
						free(conn->dh);
						free(conn);
						return EXIT_SUCCESS;
					}
					else if (strcmp(argv[i], "-z") == 0)
					{
						sscanf_s(argv[i + 1], "%d", &(conn->dh->waveSize));
						i++;
					}
					else if (strcmp(argv[i], "-x") == 0)
					{
						sscanf_s(argv[i + 1], "%lf", &(conn->dh->sampleRate));
						i++;
					}
					else
					{
						printf_s("Unknown option: %s\n", argv[i]);
						free(conn->dh);
						free(conn);
						return EXIT_SUCCESS;
					}
				}
			}
			else
			{
				SA_Log("Needs a comand line args", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
				return EXIT_SUCCESS;
			}
			switch (conn->mode)
			{
			case 0:
				SA_Log("No mode selected", LOG_MAIN, LOG_CLASS_ERROR, logOutputMethod);
				break;
			case 1:
				SA_Init(conn);
				SA_Server(conn);
				break;
			case 2:
				SA_Init(conn);
				SA_Client(conn);
				break;
			}
			if (conn != NULL)
			{
				getchar();
				SA_Close(conn);
			}
			SA_AudioClose();
		}
		SA_Log("Program exit", LOG_MAIN, LOG_CLASS_INFO, logOutputMethod);
		free(conn);
	}
	return EXIT_SUCCESS;
}