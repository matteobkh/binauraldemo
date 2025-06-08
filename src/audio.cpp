#include <iostream>
#include <algorithm>

#include "audio.h"
#include "filemanager.h"

#define FRAMES_PER_BUFFER  (256)

static PaStream* stream;

// -- AudioEngine struct defs --
AudioEngine::AudioEngine(const int sr, AudioFileData aData, float vol)
    : sampleRate(sr), audioData(aData), masterVolume(vol)
{
    std::cout << "AudioEngine created: sampleRate = " 
        << sampleRate << std::endl;
}

void AudioEngine::processAudio(float& l, float& r) {
    if (play) {
        if (index / audioData.nChannels >= audioData.frames) {
            if (!loop)
                play = false;
            index = 0;
        }
    }
    panner.process(audioData.samples[index], l, r);
    l *= masterVolume;
    r *= masterVolume;
    if (play) { index += audioData.nChannels; }
}

// Where audio processing happens for each buffer
static int paCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData ) 
{
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    for(i=0; i<framesPerBuffer; i++) {
        float left = 0.;
        float right = 0.;

        if (FileManager::fileLoaded)
            engine->processAudio(left, right);

        *out++ = left; /* left */
        *out++ = right;  /* right */
    }

    return paContinue;
}

static void StreamFinished( void* userData ) {
    printf("Stream Completed\n");
}

bool openAudio(PaDeviceIndex index, AudioEngine& audioEngine) {
    PaStreamParameters outputParameters;

    outputParameters.device = index;
    if (outputParameters.device == paNoDevice) {
        return false;
    }

    const PaDeviceInfo* pInfo = Pa_GetDeviceInfo(index);
    if (pInfo != 0)
    {
        printf("Output device name: '%s'\r", pInfo->name);
    }

    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaError err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        audioEngine.sampleRate,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        paCallback,
        &audioEngine    // pointer to audio engine to access audio objects
        );

    if (err != paNoError)
    {
        /* Failed to open stream to device !!! */
        return false;
    }

    err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);

    if (err != paNoError)
    {
        Pa_CloseStream( stream );
        stream = 0;

        return false;
    }

    return true;
}

bool closeAudio() {
    if (stream == 0)
        return false;

    PaError err = Pa_CloseStream( stream );
    stream = 0;

    return (err == paNoError);
}


bool startAudio() {
    if (stream == 0)
        return false;

    PaError err = Pa_StartStream( stream );

    return (err == paNoError);
}

bool stopAudio() {
    if (stream == 0)
        return false;

    PaError err = Pa_StopStream( stream );

    return (err == paNoError);
}