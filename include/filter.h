#ifndef FILTER_H
#define FILTER_H

#ifndef M_PI
#define M_PI (3.14159265)
#endif

// Mono single-pole IIR LPF
class LPF {
private:
    float pole = 0;
    float alpha = 0;
public:
    int sampleRate;
    float cutoff;

    // Update filter coefficient when frequency changes
    inline void updateAlpha() {
        alpha = (2.0f * M_PI * cutoff) / (2.0f * M_PI * cutoff + sampleRate);
    }

    inline void updateCutoff(float freq) { cutoff = freq; updateAlpha(); }

    inline float processMono(float input) {
        pole = pole + alpha * (input - pole);
        return pole;
    }

    LPF(int sr = 44100.0f, float freq = 20000.0f) : sampleRate(sr), cutoff(sr/2.0f) {
        updateAlpha();
    }
};

#endif //FILTER_H