
void SA_Init(saConnection *conn);

void SA_Server(saConnection *conn);

void SA_Client(saConnection *conn);

void SA_Close(saConnection *conn);

void SA_SetVolumeModifier(float vol);

float SA_GetVolumeModifier();

void SA_ListAllAudioDevices();

saConnection *SA_Setup(int device, const char *host, int mode, int port, int testMode, int channel, float volMod, int waveSize, double sampleRate);

void SA_Shutdown(saConnection *conn);

const char *SA_GetStats(saConnection *conn);