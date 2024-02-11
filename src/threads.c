#include "config.h"
#if defined(WINDOWS)
void* SA_ThreadCreate(void* func, void* param)
{
	return (void*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, param, 0, NULL);
}

void SA_ThreadClose(void* hThread)
{
	TerminateThread((HANDLE)hThread, 0);
	CloseHandle((HANDLE)hThread);
	hThread = NULL;
}

void SA_ThreadJoin(void* hThread)
{
	WaitForSingleObject((HANDLE)hThread, INFINITE);
}

void SA_ProcessSetPriority()
{
	HANDLE hProcess = GetCurrentProcess();
	SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
}
void SA_Sleep(size_t ms)
{
	Sleep((DWORD)ms);
}
#elif defined(LINUX)
#include <pthread.h>
#include <unistd.h>
void* SA_ThreadCreate(void* func, void* param)
{
	pthread_t thread;
	pthread_create(&thread, NULL, func, param);
	return (void*)thread;
}
void SA_ThreadClose(void* hThread)
{
	pthread_t thread = (pthread_t)hThread;
	pthread_cancel(thread);
}
void SA_ThreadJoin(void* hThread)
{
	pthread_t thread = (pthread_t)hThread;
	pthread_join(thread, NULL);
}
void SA_ProcessSetPriority()
{
	nice(-20);
}
void SA_Sleep(size_t ms)
{
	usleep(ms * 1000);
}
#endif
