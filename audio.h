#include "portaudio/include/portaudio.h"
#include <stdio.h>

typedef struct audioBuffer
{
    void *data;
    struct audioBuffer *next;
    struct audioBuffer* prev;
} audioBuffer;

extern void *audioDataFrame;

extern audioBuffer *head;

extern unsigned short int testMode;

extern unsigned short int barMode;

extern float volMod;

void checkErr(PaError err);

int clientCallback(
    const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
    void *userData);

int serverCallback(
    const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
    void *userData);

void stopStream(PaStream *stream);

void startStream(PaStream *stream);

void initAudio();

void closeAudio();

void listAudioDevices();

PaStream *setupStream(size_t device, size_t channel, double sampleRate, size_t waveSize, unsigned short asServer);

void shutdownStream(PaStream *stream);