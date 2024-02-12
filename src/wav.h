typedef struct wavHeader {
	char riff[4];
	int32_t size;
	char wave[4];
	char fmt[4];
	int32_t fmtSize;
	int16_t format;
	int16_t channels;
	int32_t sampleRate;
	int32_t byteRate;
	int16_t blockAlign;
	int16_t bitsPerSample;
	char data[4];
	int32_t dataSize;
} wavHeader;

wavHeader* SA_WavCreateHeader(int32_t sampleRate, int16_t bitsPerSample, int16_t channels, int32_t dataSize);

FILE* SA_WavCreateFile(wavHeader* header, const char* path);

void SA_WavWriteData(FILE* file, float* data, int32_t numSamples, int rounds);

void SA_WavCloseFile(FILE* file);

void SA_WavFreeHeader(void* header);