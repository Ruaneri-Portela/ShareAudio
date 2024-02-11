void SA_Init(saConnection* conn);

void SA_Server(saConnection* conn);

void SA_Client(saConnection* conn);

void SA_Close(saConnection* conn);

void SA_SetVolumeModifier(float vol);

float SA_GetVolumeModifier();

void SA_ListAllAudioDevices();

const char* SA_ListAllAudioDevicesStr(saConnection* conn);

void SA_Free(void* data);

const char *SA_Version();

saConnection* SA_Setup(int device, const char* host, int mode, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);

const char* SA_GetStats(saConnection* conn);

void SA_SetLogNULL();

void SA_SetLogFILE(const char* filename, int debug);

void SA_SetLogCONSOLE(int debug);

int SA_TestDLL();

const char* SA_ReadLastMsg();

int SA_SendMsg(const char* msg);

void SA_InitWavRecord(saConnection* conn, const char* path);

void SA_CloseWavRecord();

void* SA_GetWavFileP();

void SA_SetWavFileP(FILE* file);