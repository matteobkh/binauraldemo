#ifndef AUDIO_H
#define AUDIO_H

#include <atomic>
#include <vector>
#include <random>

#include "portaudio.h"

#include "filemanager.h"
#include "binaural.h"

// Struct that contains all audio objects used in the program for easy access
struct AudioEngine {
    const int sampleRate;
    AudioFileData audioData;
    bool play = false;
    bool loop = false;
    int index = 0;

    BinauralPanner panner;
    
    // room for more objects

    float masterVolume; // Master volume

    // Constructor, please specify sample rate
    AudioEngine(const int sr, AudioFileData aData, float vol = 1.0f);
    
    void processAudio(float& l, float& r);
};

// Handles PortAudio initialization
class ScopedPaHandler
{
public:
    ScopedPaHandler()
        : _result(Pa_Initialize())
    {
    }
    ~ScopedPaHandler()
    {
        if (_result == paNoError)
        {
            Pa_Terminate();
        }
    }

    PaError result() const { return _result; }

private:
    PaError _result;
};

// Control audio stream
bool openAudio(PaDeviceIndex index, AudioEngine& audioEngine);
bool closeAudio();
bool startAudio();
bool stopAudio();

#endif // AUDIO_H