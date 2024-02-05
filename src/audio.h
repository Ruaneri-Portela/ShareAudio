#include <portaudio.h>
#include <stdlib.h>

typedef struct audioDevices
{
	const PaDeviceInfo **devices;
	int numDevices;
} audioDevices;

typedef struct audioBuffer
{
	void *data;
	struct audioBuffer *next;
	struct audioBuffer *prev;
} audioBuffer;

extern char *audioDataFrame;

extern audioBuffer *head;

extern unsigned short int testMode;

int SA_AudioClientCallback(
	const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
	void *userData);

int SA_AudioServerCallback(
	const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
	void *userData);

void SA_AudioCloseStream(PaStream *stream);

void SA_AudioStartStream(PaStream *stream);

void SA_AudioInit();

void SA_AudioClose();

audioDevices SA_GetAllDevices();

PaStream *SA_AudioOpenStream(size_t device, unsigned short asServer, void *configs);

void SA_AudioCloseStream(PaStream *stream);