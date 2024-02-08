#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>

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

FILE* wavFile = NULL;

wavHeader* headerWav = NULL;

int rounds = 1;

wavHeader* SA_WavCreateHeader(int32_t sampleRate, int16_t bitsPerSample, int16_t channels, int32_t dataSize) {
	wavHeader* header = (wavHeader*)malloc(sizeof(wavHeader));
	if (header == NULL) {
		return NULL;
	}
	memcpy_s(header->riff, 4, "RIFF", 4);
	header->size = dataSize + 36;
	memcpy_s(header->wave, 4, "WAVE", 4);
	memcpy_s(header->fmt, 4, "fmt ", 4);
	header->fmtSize = 16;
	header->format = 1;
	header->channels = channels;
	header->sampleRate = sampleRate;
	header->bitsPerSample = bitsPerSample;
	header->byteRate = (sampleRate * channels * bitsPerSample) / 8;
	header->blockAlign = (channels * bitsPerSample) / 8;
	memcpy_s(header->data, 4, "data", 4);
	header->dataSize = dataSize;
	return header;
}

FILE* SA_WavCreateFile(wavHeader *header, const char* path) {
	FILE* file = fopen(path, "wb");
	fwrite(header, sizeof(wavHeader), 1, file);
	return file;
}

void SA_WavUpdateSize(FILE* file, int32_t dataSize) {
	fseek(file, 40, SEEK_SET);
	fwrite(&dataSize, sizeof(int32_t), 1, file);
	fseek(file, 4, SEEK_SET);
	int32_t fileSize = 36 + dataSize;
	fwrite(&fileSize, sizeof(int32_t), 1, file);
	fseek(file, 0, SEEK_END);
}

void SA_WavWriteData(FILE* file, float* data, int32_t numSamples) {
	for (int i = 0; i < numSamples; ++i) {
		PaSampleFormat format = paFloat32;
		PaSampleFormat destFormat = paInt32;
		int32_t sample = (int32_t)(data[i] * 2147483647.0f);
		fwrite(&sample, sizeof(sample), 1, file);
	}
	SA_WavUpdateSize(file, (numSamples * rounds)*4);
	rounds++;
}

void SA_WavCloseFile(FILE* file) {
	fclose(file);
}

void SA_WavFreeHeader(void* header) {
	free(header);
}
