#include "audio.h"

#include <memory>
#include <iostream>
#include <algorithm>

static SDL_AudioSpec g_have = {};

Audio::~Audio() {
    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
}

static void SDLCALL audioCallback(void* userdata, Uint8* stream, int len) {
    SDL_assert(g_have.format  == AUDIO_U8);
    SDL_assert(g_have.channels == 1);

    auto* beep = static_cast<BeepState*>(userdata);

    SDL_memset(stream, g_have.silence, len);

    const int sampleRate = g_have.freq ? g_have.freq : 44100;
    const int channels = g_have.channels ? g_have.channels : 1;

    const int frames = len / channels;

    const float rampMs = 10.0f;
    const int   rampN  = std::max(1, int(sampleRate * rampMs / 1000.0f));
    const float step   = 1.0f / rampN;

    const float desired = beep->isBeeping ? 1.0f : 0.0f;

    const int frequency = 440;
    const int samplesPerCycle = std::max(1, sampleRate / frequency);
    const int halfCycle = samplesPerCycle / 2;
    const int amp = 5;

    Uint8* out = stream;
    for (int i = 0; i < frames; ++i) {
        if (beep->gain < desired)      beep->gain = std::min(beep->gain + step, 1.0f);
        else if (beep->gain > desired) beep->gain = std::max(beep->gain - step, 0.0f);

        int s = (beep->phase < halfCycle) ? (128 + amp) : (128 - amp);
        int mixed = 128 + int((s - 128) * beep->gain);

        out[i] = (Uint8)std::clamp(mixed, 0, 255);

        beep->phase = (beep->phase + 1) % samplesPerCycle;
    }
}

int Audio::init() {
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = &beepState;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &g_have, 0);
    if (audioDevice == 0) {
        std::printf("SDL_OpenAudioDevice error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_PauseAudioDevice(audioDevice, 0);
    return 0;
}

void Audio::setIsBeeping(const bool beeping) {
    if (audioDevice) {
        SDL_LockAudioDevice(audioDevice);
    }

    beepState.isBeeping = beeping;

    if (audioDevice) {  
        SDL_UnlockAudioDevice(audioDevice);
    }
}
