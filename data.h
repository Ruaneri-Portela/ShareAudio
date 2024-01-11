#include <stdlib.h>
/**
 * @file data.h
 * @brief Data frame functions.
 */
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

/**
 * @brief Enumeration of expected headers in the data frame.
 */
typedef enum enumHeader dataHeader;

/**
 * @brief Struct representing a handshake data structure.
 *
 * This struct contains information related to a handshake, including the header, channel, sample rate, and wave size.
 */
typedef struct dataHandshake
{
    dataHeader header;
    size_t channel;
    double sampleRate;
    size_t waveSize;
} dataHandshake;

extern dataHandshake *dh;

/**
 * @brief Creates a data frame from an array of float data.
 *
 * This function takes an array of float data and converts it into a data frame.
 * The data frame format consists of a header followed by the actual data.
 *
 * @param data The array of float data.
 * @param waveSize The size of the data array.
 * @return A pointer to the created data frame.
 */
char *createDataFrame(const float *data, size_t waveSize);

/**
 * @brief Retrieves the wave frame from a data frame.
 *
 * This function extracts the wave frame from a given data frame.
 * The wave frame contains the actual data without the header.
 *
 * @param dataFrame The data frame.
 * @return A pointer to the wave frame.
 */
float *getWaveFrame(const char *dataFrame);

/**
 * @brief Retrieves the sample size from a data frame.
 *
 * This function returns the sample size of the data frame.
 * The sample size is the number of elements in the wave frame.
 *
 * @param dataFrame The data frame.
 * @return The sample size.
 */
const int getSampleSize(const char *dataFrame);

/**
 * @brief Retrieves the handshake information from a data frame.
 *
 * This function extracts the handshake information from a given data frame.
 * The handshake information is stored in the header of the data frame.
 *
 * @param dataFrame The data frame.
 * @return The handshake information.
 */
dataHandshake getHandShake(char *dataFrame);