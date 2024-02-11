typedef enum dataHeader
{
	NULLHEADER = 0x00,
	HANDSHAKE = 0x01,
	AUTH = 0x02,
	DATA = 0x03,
	NULLDATA = 0x04,
	DATAMSG = 0x05,
	AUTHCHANGE = 0xFD,
	DISCONNECT = 0xFE,
	END = 0xFF,
} dataHeader;

typedef struct dataHandshake
{
	dataHeader header;
	int channel;
	double sampleRate;
	int waveSize;
	float volMod;
	int testMode;
	size_t sessionPacket;
	size_t totalPacketSrv;
} dataHandshake;

typedef struct saConnection
{
	void** thread;
	void* audio;
	int port;
	int device;
	const char* host;
	int mode;
	int exit;
	dataHandshake* dh;
} saConnection;

void SA_DataCopyAudio(float* in, float* out, size_t size, float volMod, size_t testMode);

size_t SA_DataGetDataFrameSize(dataHandshake* dhData);

void SA_DataPutOrderDataFrame(char* dataFrame, size_t value, dataHandshake* dhData);

size_t SA_DataGetOrderDataFrame(char* dataFrame, dataHandshake* dhData);

char* SA_DataCreateDataFrame(const float* data, dataHandshake* dhData, unsigned short int testmode);

float* SA_DataGetWaveData(const char* dataFrame);

size_t SA_DataGetWaveSize(const char* dataFrame);

size_t SA_DataGetDelayInterFrames(dataHandshake* dhData);

char* SA_DataConcatString(const char* original, const char* toCat);

unsigned short int SA_DataDetectIsIp(const char* host, size_t asServer);

void SA_DataCopyStr(char* target, const char* input);

void SA_DataRevcProcess(int* rounds, char** msgStream, char* msgLocal, char** msg);