#ifndef BINAURAL_H
#define BINAURAL_H

#include "filter.h"
#include "delay.h"

#define MAX_DELAY (44)
#define MIN_CUTOFF (8000.0f)
#define DELTA_CUTOFF (12000.0f) //12k + 8k = 20kHz
#define MIN_AMP (0.2512f) //-12dBFS
#define DELTA_AMP (0.2067f) //plus MIN_AMP = 0.7079 -> -3dB

class BinauralPanner {
private:
    DelayLine delay = DelayLine(MAX_DELAY + 2);
    LPF filterL, filterR;
public:
    float angle = 0.0f;
    float dist = 0.333f;
    
    void process(float in, float& l, float& r);

    void update(float angle);

    void drawInterface();
};

#endif //BINAURAL_H