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

extern const char* fileLogName;

extern logOutput logOutputMethod;

void SA_Log(const char* msg, logClass class, logLevel level);