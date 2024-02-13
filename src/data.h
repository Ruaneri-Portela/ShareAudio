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

typedef struct audioDevices
{
	const PaDeviceInfo** devices;
	int numDevices;
} audioDevices;

typedef struct audioBuffer
{
	void* data;
	struct audioBuffer* next;
	struct audioBuffer* prev;
} audioBuffer;


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
	unsigned char iv[128];
} dataHandshake;

typedef struct saConnection
{
	char data[DATASIZE + 3];
	char* msg;

	FILE* wavFile;
	wavHeader* headerWav;
	int rounds;

	char* audioDataFrame;
	audioBuffer* head;

	dataHandshake* dh;
	const char* host;
	void* thread;
	void* audio;
	int device;
	int port;
	int mode;
	int runCode;
	unsigned char key[256];
} saConnection;

extern const unsigned char confirmConn[2];

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

void SA_DataCopyStr(char* target, const char* input, size_t sizeMax);

void SA_DataRevcProcess(size_t* rounds, char** msgStream, char* msgLocal, char** msg);