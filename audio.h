#include <SDL.h>

struct BeepState {
    bool isBeeping = false;
    int phase = 0;
    float gain = 0.0f;
    float target = 0.0f;
};

class Audio {

    public:
        ~Audio();
        int init();
        void setIsBeeping(const bool beeping);

    private:
        BeepState beepState;
        SDL_AudioDeviceID audioDevice;
};