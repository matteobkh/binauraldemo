#ifndef DELAY_H
#define DELAY_H

#include <vector>

class DelayLine {
private:
    int writeIndex = 0;
    std::vector<float> buffer;
public:
    inline float read(int delayTimeSamples) {
        return buffer[(writeIndex+delayTimeSamples) % buffer.size()]; 
    }
    inline void write(float input) { 
        buffer[writeIndex] = input; 
        writeIndex = (writeIndex+1) % buffer.size();
    }
    // Create new empty delay line, size in samples
    inline DelayLine(int size) : buffer(size,0.0f) {}
};

#endif //DELAY_H