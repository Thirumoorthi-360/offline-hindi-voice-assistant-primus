#pragma once
#include <string>

class TTS {
public:
    TTS(float gain = 1.0, int speed = 130, int pitch = 22);
    void speak(const std::string& text);
    void setTone(float gain, int speed, int pitch);

private:
    float gain;
    int   speed;
    int   pitch;
    void  processWav(const std::string& filename);
};
