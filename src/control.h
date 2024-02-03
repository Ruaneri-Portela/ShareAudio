
void SA_Init(saConnection* conn);

void SA_Server(saConnection* conn);

void SA_Client(saConnection* conn);

void SA_Close(saConnection* conn);

void SA_SetVolumeModifier(float vol);

float SA_GetVolumeModifier();

void SA_ListAllAudioDevices();
