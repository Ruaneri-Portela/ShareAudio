#include <stdio.h>
#include <string.h>

#include "ShareAudio.h"

char input[2048];

int main(int argc, char* argv[])
{
	SA_TestDLL();
	SA_SetLogCONSOLE(1);
	saConnection* conn = SA_Setup(-1, NULL, 0, 9950, 0, 2, -1, 2048, -1);
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
				SA_SetLogFILE("shareaudio.log", 0);
			}
			else if (strcmp(argv[i], "-i") == 0)
			{
				printf_s("ShareAudio\n\tA easy and light way to share your between your computers\n\tVer: %s\n", VERSION);
			}
			else if (strcmp(argv[i], "-r") == 0)
			{
				SA_ListAllAudioDevices(NULL);
				goto EXIT;
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
				goto EXIT;
			}
			else if (strcmp(argv[i], "-he") == 0)
			{
				printf_s("Examples:\n");
				printf_s("[1] ShareAudio -s -p 9950 -a 0.0.0.0 -d 0\n");
				printf_s("\tStart the program in server mode, listening on port 9950, on all interfaces, using audio device 0\n");
				printf_s("[2] ShareAudio -c - p 9950 -a 192.168.1.1 -d 0\n");
				printf_s("\tStart the program in client mode, connecting to 192.168.1.1 in port 9950, using audio device 0\n");
				goto EXIT;
			}
			else if (strcmp(argv[i], "-ht") == 0)
			{
				printf_s("Troubleshooting:\n");
				printf_s("[1] Allow this program on firewall in case connection is no prossible into computers\n");
				printf_s("[2] If 'Channel is not avalible' you as select non capture to server , and outup to client device\n\t Client accpets only out devices and server only capture or Loopbacks\n");
				printf_s("[3] If you have a problem with audio, try to change the audio device, some output on Windows not work propery\n\t Recommend select to outup to Windows 'Sound Mapper'\n");
				printf_s("[3] In bind error, change you port");
				goto EXIT;
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
				goto EXIT;
			}
		}
	}
	else
	{
		SA_Log("Needs a comand line args", LOG_MAIN, LOG_CLASS_ERROR);
		goto EXIT;
	}
	switch (conn->mode)
	{
	case 0:
		SA_Log("No mode selected", LOG_MAIN, LOG_CLASS_ERROR);
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
		while (conn != NULL) {
			char a = getchar();
			switch (a)
			{
			case 's':
				printf("Stats:");
				printf("Bytes played: %zd\n", conn->dh->sessionPacket - 1);
				printf("Bytes send by server: %zd\n", conn->dh->totalPacketSrv);
				printf("Packets lost: %zd\n", conn->dh->totalPacketSrv - (conn->dh->sessionPacket - 1));
				printf("Packets percent lost: %.2lf\n", (double)(conn->dh->totalPacketSrv - (conn->dh->sessionPacket - 1)) / (double)(conn->dh->totalPacketSrv));
				printf("Bandwidth: %.2f MB\n", ((conn->dh->sessionPacket - 1) * sizeof(float)) / 1024.0f);
				break;
			case 'q':
				printf("Quit...");
				SA_Close(conn);
				conn = NULL;
				break;
			case 'm':
				printf("Enter a message: ");
				getchar();
				fgets(input, 2048, stdin);
				SA_SendMsg(input);
				break;
			case 'r':
				printf("Show last message: ");
				printf("%s\n", SA_ReadLastMsg());
				break;
			case 'z':
				if (SA_GetWavFileP() == NULL) {
					printf("Start record\n");
					SA_InitWavRecord(conn, "record.wav");
				}
				else {
					printf("Stop record\n");
					SA_CloseWavRecord();
				}
				break;
			default:
				break;
			}
		}
	}
	SA_Shutdown(conn);
	SA_Log("Program exit", LOG_MAIN, LOG_CLASS_INFO);
EXIT:
	return EXIT_SUCCESS;
}