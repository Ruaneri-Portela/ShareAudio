#include "portaudio/include/portaudio.h"
#include <stdio.h>
/**
 * @file audio.h
 * @brief Header file containing declarations for audio functions and structures.
 */

/**
 * @struct audioBuffer
 * @brief Structure representing a linked list node that stores audio data to be played.
 */
typedef struct audioBuffer
{
    void *data; /**< Pointer to the audio data */
    void *next; /**< Pointer to the next audioBuffer node */
} audioBuffer;

extern void *audioDataFrame; /**< Pointer dataframe, use on server context */

extern audioBuffer *head; /**< Pointer to the head of the audioBuffer linked list */

extern PaStream* globalStream; /**< Pointer to the PortAudio stream */

extern int testMode; /**< Flag indicating whether test mode is enabled */

extern int barMode; /**< Flag sample bar mode is enabled */

extern float volMod ; /** < Volume Multiplifier/

/**
 * @brief Checks for errors in PortAudio functions and prints error message if necessary.
 * @param err The PortAudio error code
 */
void checkErr(PaError err);

/**
 * @brief Callback function for the client mode.
 * @param inputBuffer Pointer to the input audio buffer
 * @param outputBuffer Pointer to the output audio buffer
 * @param framesPerBuffer Number of frames per buffer
 * @param timeInfo Pointer to time information for the callback
 * @param statusFlags Status flags for the callback
 * @param userData Pointer to user data
 * @return PaStreamCallbackResult indicating the result of the callback
 */
int clientCallback(
    const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
    void *userData);

/**
 * @brief Callback function for the server mode.
 * @param inputBuffer Pointer to the input audio buffer
 * @param outputBuffer Pointer to the output audio buffer
 * @param framesPerBuffer Number of frames per buffer
 * @param timeInfo Pointer to time information for the callback
 * @param statusFlags Status flags for the callback
 * @param userData Pointer to user data
 * @return PaStreamCallbackResult indicating the result of the callback
 */
int serverCallback(
    const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
    void *userData);

/**
 * @brief Stops the given PortAudio stream.
 * @param s Pointer to the PortAudio stream
 */
void stopStream(PaStream *s);

/**
 * @brief Starts the given PortAudio stream.
 * @param s Pointer to the PortAudio stream
 */
void startStream(PaStream *s);

/**
 * @brief Initializes the audio system.
 */
void initAudio();

/**
 * @brief Closes the audio system.
 */
void closeAudio();

/**
 * @brief Lists the available audio devices.
 */
void listAudioDevices();

/**
 * @brief Sets up a PortAudio stream with the specified parameters.
 * @param device Index of the audio device to use
 * @param channel Number of audio channels
 * @param sampleRate Sample rate of the audio stream
 * @param waveSize Size of the audio wave buffer
 * @param asServer Flag indicating whether the stream should behave as a server or client
 * @return Pointer to the created PortAudio stream
 */
PaStream *setupStream(size_t device, size_t channel, double sampleRate, size_t waveSize, unsigned short asServer);

/**
 * @brief Shuts down the given PortAudio stream.
 * @param stream Pointer to the PortAudio stream
 */
void shutdownStream(PaStream *stream);