#include <windows.h>

typedef struct connectParam
{
    int port;
    char addr[16];
    char *host;
} connectParam;

extern HANDLE closeThread;

HANDLE initNet(int port, char addr[], char *host, size_t asClient);

void closeNet(void *hThread);
