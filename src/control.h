void SA_Init(saConnection* conn);

void SA_Server(saConnection* conn);

void SA_Client(saConnection* conn);

void SA_Close(saConnection* conn);

void SA_SetVolumeModifier(float vol,saConnection* conn);

float SA_GetVolumeModifier(saConnection* conn);

void SA_ListAllAudioDevices(saConnection* conn);

const char* SA_ListAllAudioDevicesStr(saConnection* conn);

void SA_Free(void* data);

const char *SA_Version();

saConnection* SA_Setup(int device, const char* host, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);

const char* SA_GetStats(saConnection* conn);

void SA_SetLogNULL();

void SA_SetLogFILE(const char* filename, int debug);

void SA_SetLogCONSOLE(int debug);

int SA_TestDLL();

const char* SA_ReadLastMsg(saConnection* conn);

int SA_SendMsg(const char* msg , saConnection* conn);

void SA_InitWavRecord(saConnection* conn, const char* path);

void SA_CloseWavRecord(saConnection* conn);

void* SA_GetWavFilePtr(saConnection* conn);

void SA_SetKey(saConnection* conn, const char* key);

void SA_SetMode(saConnection* conn, int mode);