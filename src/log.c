#include "config.h"
#include <stdio.h>
#include <stdlib.h>

enum logClassEnum
{
	LOG_NET,
	LOG_AUDIO,
	LOG_CRIPTO,
	LOG_MAIN
};
enum logLevelEnum
{
	LOG_CLASS_ERROR,
	LOG_CLASS_WARNING,
	LOG_CLASS_INFO,
	LOG_CLASS_DEBUG
};
enum logOutputEnum
{
	LOG_OUTPUT_CONSOLE,
	LOG_OUTPUT_FILE,
	LOG_OUTPUT_CONSOLE_DEBUG,
	LOG_OUTPUT_FILE_DEBUG,
	LOG_OUTPUT_NULL
};
typedef enum logClassEnum logClass;
typedef enum logLevelEnum logLevel;
typedef enum logOutputEnum logOutput;

logOutput logOutputMethod = LOG_OUTPUT_NULL;

const char* fileLogName = "log.txt";

static FILE* fileLog = NULL;

static const char* SA_LogEnumClass(logClass class)
{
	switch (class)
	{
	case LOG_NET:
		return "NET";
	case LOG_AUDIO:
		return "AUDIO";
	case LOG_CRIPTO:
		return "CRIPTO";
	case LOG_MAIN:
		return "MAIN";
	default:
		return "UNKNOWN";
	}
}

static const char* SA_LogEnumToLevel(logLevel level, int toTerminal)
{
	if (toTerminal == 1)
	{
		switch (level)
		{
		case LOG_CLASS_ERROR:
			return "\x1B[31m[ERROR";
		case LOG_CLASS_WARNING:
			return "\x1B[33m[WARNING";
		case LOG_CLASS_INFO:
			return "\x1B[34m[INFO";
		case LOG_CLASS_DEBUG:
			return "\x1B[37m[DEBUG";
		default:
			return "\x1B[0mUNKNOWN";
		}
	}
	else
	{
		switch (level)
		{
		case LOG_CLASS_ERROR:
			return "ERROR";
		case LOG_CLASS_WARNING:
			return "WARNING";
		case LOG_CLASS_INFO:
			return "INFO";
		case LOG_CLASS_DEBUG:
			return "DEBUG";
		default:
			return "UNKNOWN";
		}
	}
}

EXPORT void SA_Log(const char* msg, logClass class, logLevel level)
{
	switch (logOutputMethod)
	{
	case LOG_OUTPUT_CONSOLE:
	case LOG_OUTPUT_CONSOLE_DEBUG:
		if (logOutputMethod != LOG_OUTPUT_CONSOLE_DEBUG && level == LOG_CLASS_DEBUG)
		{
			break;
		}
		else {
			printf_s("%s][%s] %s\n\x1B[0m", SA_LogEnumToLevel(level, 1), SA_LogEnumClass(class), msg);
		}
		break;
	case LOG_OUTPUT_FILE:
	case LOG_OUTPUT_FILE_DEBUG:
		if (logOutputMethod != LOG_OUTPUT_CONSOLE_DEBUG && level == LOG_CLASS_DEBUG)
		{
			break;
		}
		else {
			fopen_s(&fileLog, fileLogName, "a");
			if (fileLog != NULL)
			{
				fprintf(fileLog, "[%s][%s] %s\n", SA_LogEnumToLevel(level, 0), SA_LogEnumClass(class), msg);
				fclose(fileLog);
			}
			else
			{
				printf_s("Error opening file %s\n", fileLogName);
			}
		}
		break;
	default:
		break;
	}
	if (level == LOG_CLASS_ERROR)
	{
		exit(EXIT_FAILURE);
	}
}