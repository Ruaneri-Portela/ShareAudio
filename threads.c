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
#endif