#include <iostream>
#include <SDL.h>

#include "window.h"
#include "audio.h"
#include "chip8.h"
#include "arg_parser.h"

const double CPU_TICK_DURATION = 1.0 / 500.0;
const double TIMER_TICK_DURATION = 1.0 / 60.0;
const double DISPLAY_DRAW_DURATION = 1.0 / 60.0;
const double INPUT_READ_DURATION = 1.0 / 60.0;

double freq = SDL_GetPerformanceFrequency();

double hiresTime() {
    return (double)SDL_GetPerformanceCounter() / freq;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    Settings settings = ArgParser::parse(argc, argv);

    if (settings.rom.size() == 0) {
        std::cerr << "No ROM supplied\n";
        return 1;
    }

    Window window;
    if (window.init() == 1) {
        SDL_Quit();

        return 1;
    };

    Audio audio;
    if (audio.init() == 1) {
        SDL_Quit();

        return 1;
    };

    Chip8 chip8(settings);
    chip8.init();

    double previousTime = hiresTime();
    SDL_Event event;
    bool quit = false;
    double cpuAccumulator = 0;
    double timerAccumulator = 0;
    double displayAccumulator = 0;
    double inputDelayTimer = 0;

    int currentIsHires = chip8.isHires();

    while (!quit) {
        double now = hiresTime();
        double delta = std::clamp(now - previousTime, 0.0, 0.25);
        previousTime = now;

        cpuAccumulator += delta;
        timerAccumulator += delta;
        displayAccumulator += delta;
        inputDelayTimer += delta;

        while (SDL_PollEvent(&event))  {
            SDL_EventType type = (SDL_EventType)event.type;
            SDL_KeyCode sym = (SDL_KeyCode)event.key.keysym.sym;

            if (type == SDL_QUIT || (type == SDL_KEYDOWN && sym == SDLK_ESCAPE)) {
                quit = true;
                break;
            }
        }

        if (inputDelayTimer >= INPUT_READ_DURATION) {
            const Uint8* keyStates = SDL_GetKeyboardState(NULL);

            chip8.keypad[0x1] = keyStates[SDL_SCANCODE_1];
            chip8.keypad[0x2] = keyStates[SDL_SCANCODE_2];
            chip8.keypad[0x3] = keyStates[SDL_SCANCODE_3];
            chip8.keypad[0xC] = keyStates[SDL_SCANCODE_4];

            chip8.keypad[0x4] = keyStates[SDL_SCANCODE_Q];
            chip8.keypad[0x5] = keyStates[SDL_SCANCODE_W];
            chip8.keypad[0x6] = keyStates[SDL_SCANCODE_E];
            chip8.keypad[0xD] = keyStates[SDL_SCANCODE_R];

            chip8.keypad[0x7] = keyStates[SDL_SCANCODE_A];
            chip8.keypad[0x8] = keyStates[SDL_SCANCODE_S];
            chip8.keypad[0x9] = keyStates[SDL_SCANCODE_D];
            chip8.keypad[0xE] = keyStates[SDL_SCANCODE_F];

            chip8.keypad[0xA] = keyStates[SDL_SCANCODE_Z];
            chip8.keypad[0x0] = keyStates[SDL_SCANCODE_X];
            chip8.keypad[0xB] = keyStates[SDL_SCANCODE_C];
            chip8.keypad[0xF] = keyStates[SDL_SCANCODE_V];

            inputDelayTimer -= INPUT_READ_DURATION;
        }

        while (cpuAccumulator >= CPU_TICK_DURATION) {
            chip8.cycle();
            cpuAccumulator -= CPU_TICK_DURATION;
        }

        while (timerAccumulator >= TIMER_TICK_DURATION) {
            chip8.tickTimers();
            timerAccumulator -= TIMER_TICK_DURATION;
        }

        if (currentIsHires != chip8.isHires()) {
            currentIsHires = chip8.isHires();

            window.setLogicalSize(chip8.screenWidth(), chip8.screenHeight());
        }

        if (chip8.isHalted()) {
            quit = true;
        }

        if (chip8.displayBufferUpdated && displayAccumulator >= DISPLAY_DRAW_DURATION) {
            window.draw(chip8.getDisplayBuffer());
            chip8.displayBufferUpdated = false;
            displayAccumulator = 0;
        }
        
        audio.setIsBeeping(chip8.isBeeping());

        SDL_Delay(0);
    }

    SDL_Quit();

    return 0;
}
