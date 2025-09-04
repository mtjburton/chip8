#pragma once

#include <SDL.h>
#include <array>

class Window {

    public:
        ~Window();
        int init();
        void draw(const std::array<std::array<uint8_t, 128>, 64>& displayBuffer);
        void terminalDraw(const std::array<std::array<uint8_t, 128>, 64>& displayBuffer);
        void setLogicalSize(const int width, const int height);

    private:

        SDL_Window* pWindow = nullptr;
        SDL_Renderer* pRenderer = nullptr;
        SDL_Texture*  pTexture  = nullptr;

        const int SCREEN_WIDTH = 1280;
        const int SCREEN_HEIGHT = 640;
        const int SCALE = SCREEN_WIDTH / 64;

        int logicalWidth = 64;
        int logicalHeight = 32;
        
        static constexpr SDL_Color BG_COLOUR = { 200, 195, 190, 255 };
        static constexpr SDL_Color FG_COLOUR = { 0, 0, 0, 255 };

        Uint32 fgPacked = 0;
        Uint32 bgPacked = 0;
};