#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
void* SA_ThreadCreate(void* func, void* param)
{
	return (void*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, param, 0, NULL);
}

void SA_ThreadClose(void* hThread)
{
	CloseHandle((HANDLE)hThread);
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
void SA_Sleep(unsigned int ms) {
	Sleep((DWORD)ms);
}
#else
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
	pthread_t thread = pthread_self();
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(thread, SCHED_FIFO, &param);
}
void SA_Sleep(unsigned int ms) {
	usleep(ms * 1000);
}
#endif
