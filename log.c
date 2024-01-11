#include <stdio.h>
#include <stdlib.h>

enum logClassEnum
{
	LOG_NET,
	LOG_AUDIO,
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
	LOG_OUTPUT_FILE
};
typedef enum logClassEnum logClass;
typedef enum logLevelEnum logLevel;
typedef enum logOutputEnum logOutput;

logOutput logOutputMethod;

char* fileLogName = "log.txt";

static FILE* fileLog = NULL;

static const char* enumToStringClass(logClass class)
{
	switch (class)
	{
	case LOG_NET:
		return "NET";
	case LOG_AUDIO:
		return "AUDIO";
	case LOG_MAIN:
		return "MAIN";
	default:
		return "UNKNOWN";
	}
}

static const char* enumToStringLevel(logLevel level, size_t toTerminal)
{
	if (toTerminal = 1) {
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
	else {
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

void logCat(const char* msg, logClass class, logLevel level, logOutput method)
{
	switch (method)
	{
	case LOG_OUTPUT_CONSOLE:
		printf_s("%s][%s] %s\n\x1B[0m", enumToStringLevel(level, 1), enumToStringClass(class), msg);
		break;
	case LOG_OUTPUT_FILE:
		fileLog == NULL ? fopen_s(&fileLog,fileLogName, "a") : fileLog;
		if (fileLog != NULL) {
			fprintf(fileLog, "[%s][%s] %s\n", enumToStringLevel(level, 0), enumToStringClass(class), msg);
		}
		else {
			printf_s("Error opening file %s\n", fileLogName);
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