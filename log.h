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

extern char *fileLogName;

extern logOutput logOutputMethod;

void logCat(const char *msg, logClass class, logLevel level, logOutput method);