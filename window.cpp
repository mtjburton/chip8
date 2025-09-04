#include "window.h"
#include "SDL.h"
#include <iostream>

Window::~Window() {
    if (pTexture != nullptr) { 
        SDL_DestroyTexture(pTexture);
        pTexture = nullptr;
    }

    if (pRenderer != nullptr) {
        SDL_DestroyRenderer(pRenderer);
        pRenderer = nullptr;
    }

    if (pWindow != nullptr) {
        SDL_DestroyWindow(pWindow);
        pWindow = nullptr;
    }  
}

int Window::init() {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    pWindow = SDL_CreateWindow(
        "chip8", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN);
            
    if (pWindow == nullptr) {
        std::printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        
        return 1;
    }

    pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (pRenderer == nullptr) {
        SDL_DestroyWindow(pWindow);
        pWindow = nullptr;

        std::printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        
        return 1;
    }

    SDL_RenderSetIntegerScale(pRenderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(pRenderer, logicalWidth, logicalHeight);

    pTexture = SDL_CreateTexture(pRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        128, 
        64
    );

    if (pTexture == nullptr) {
        std::printf("SDL_CreateTexture error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(pRenderer);
        pRenderer = nullptr;

        SDL_DestroyWindow(pWindow);
        pWindow = nullptr;

        return 1;
    }

    SDL_PixelFormat* pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    bgPacked = SDL_MapRGBA(pixelFormat, BG_COLOUR.r, BG_COLOUR.g, BG_COLOUR.b, BG_COLOUR.a);
    fgPacked = SDL_MapRGBA(pixelFormat, FG_COLOUR.r, FG_COLOUR.g, FG_COLOUR.b, FG_COLOUR.a);
    SDL_FreeFormat(pixelFormat);

    SDL_SetRenderDrawColor(
        pRenderer, 
        BG_COLOUR.r, 
        BG_COLOUR.g, 
        BG_COLOUR.b, 
        BG_COLOUR.a);
        
    SDL_RenderClear(pRenderer);
    SDL_RenderPresent(pRenderer);

    return 0;
}

void Window::draw(const std::array<std::array<uint8_t, 128>, 64>& buffer) {
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(pTexture, nullptr, &pixels, &pitch) != 0) {
        std::printf("SDL_LockTexture error: %s\n", SDL_GetError());
        return;
    }

    Uint8* row = static_cast<Uint8*>(pixels);
    for (int y = 0; y < logicalHeight; ++y) {
        Uint32* px = reinterpret_cast<Uint32*>(row);

        for (int x = 0; x < logicalWidth; ++x) {
            const bool on = buffer[y][x] != 0;
            px[x] = on ? fgPacked : bgPacked;
        }

        row += pitch;
    }

    SDL_UnlockTexture(pTexture);

    SDL_RenderClear(pRenderer);

    SDL_Rect src{0, 0, logicalWidth, logicalHeight};
    SDL_RenderCopy(pRenderer, pTexture, &src, nullptr);
    SDL_RenderPresent(pRenderer);
}

void Window::terminalDraw(const std::array<std::array<uint8_t, 128>, 64>& displayBuffer) {
    for (int y = 0; y < logicalHeight; ++y) {
        for (int x = 0; x < logicalWidth; ++x) {
            std::printf((displayBuffer)[y][x] ? "█" : " ");
        }
        std::printf("\n");
    }
}

void Window::setLogicalSize(const int width, const int height) {
    logicalWidth = width;
    logicalHeight = height;

    SDL_RenderSetLogicalSize(pRenderer, logicalWidth, logicalHeight);
}