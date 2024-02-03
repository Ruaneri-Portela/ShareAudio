typedef struct saConnection
{
	void* thread;
	void* audio;
} saConnection;

saConnection* SA_Server(int device, int port, const char* host);

saConnection* SA_Client(int device, int port, const char* host);

void SA_Close(saConnection* conn);

void SA_SetVolumeModifier(float vol);

float SA_GetVolumeModifier();

void SA_ListAllAudioDevices();