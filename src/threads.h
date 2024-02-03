void *SA_ThreadCreate(void *func, void *param);

void SA_ThreadClose(void *hThread);

void SA_ThreadJoin(void *hThread);

void SA_ProcessSetPriority();

void SA_Sleep(size_t ms);