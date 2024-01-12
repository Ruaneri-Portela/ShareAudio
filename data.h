#include <stdlib.h>
enum enumHeader
{
    NULLHEADER = 0x00,
    HANDSHAKE = 0x01,
    AUTH = 0x02,
    DATA = 0x03,
    AUTHCHANGE = 0xFD,
    DISCONNECT = 0xFE,
    END = 0xFF,
};

typedef enum enumHeader dataHeader;

typedef struct dataHandshake
{
    dataHeader header;
    size_t channel;
    double sampleRate;
    size_t waveSize;
} dataHandshake;

extern dataHandshake *dh;

void copyInto(float* in, float* out, size_t size, float volMod, size_t testMode);

char *createDataFrame(const float *data, size_t waveSize);

float *getWaveFrame(const char *dataFrame);

int getSampleSize(const char *dataFrame);

dataHandshake getHandShake(char *dataFrame);