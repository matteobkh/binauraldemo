#ifndef GUI_H
#define GUI_H

#include <vector>
#include <memory>
#include "audio.h"

namespace GUI {
    void mainInterface(AudioEngine& audioEngine);
    void loadFile();
}

#endif // GUI_H