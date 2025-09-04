#pragma once

#include <string>
#include <array>
#include <random>
#include <span>
#include <cstdint>

#include "settings.h"

inline constexpr size_t FONT_START = 0x50;
inline constexpr size_t BIGFONT_START = 0x100;
inline constexpr size_t ROM_START = 0x200;
inline constexpr size_t MAX_ROM_SIZE = 4096 - ROM_START;

inline constexpr std::array<uint8_t, 80> FONTSET = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

inline constexpr std::array<uint8_t, 160> BIGFONTSET = {
    // 0
    0x3C,0x66,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,
    // 1
    0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,
    // 2
    0x3C,0x66,0x06,0x06,0x0C,0x18,0x30,0x60,0x66,0x7E,
    // 3
    0x3C,0x66,0x06,0x06,0x1C,0x06,0x06,0x06,0x66,0x3C,
    // 4
    0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x0C,
    // 5
    0x7E,0x60,0x60,0x7C,0x66,0x06,0x06,0x06,0x66,0x3C,
    // 6
    0x1C,0x30,0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x3C,
    // 7
    0x7E,0x66,0x06,0x06,0x0C,0x18,0x18,0x18,0x18,0x18,
    // 8
    0x3C,0x66,0x66,0x66,0x3C,0x66,0x66,0x66,0x66,0x3C,
    // 9
    0x3C,0x66,0x66,0x66,0x3E,0x06,0x06,0x06,0x0C,0x38,
    // A
    0x18,0x3C,0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x66,
    // B
    0x7C,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0x7C,
    // C
    0x3C,0x66,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x66,0x3C,
    // D
    0x78,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0x78,
    // E
    0x7E,0x60,0x60,0x60,0x7C,0x60,0x60,0x60,0x60,0x7E,
    // F
    0x7E,0x60,0x60,0x60,0x7C,0x60,0x60,0x60,0x60,0x60,
};

struct Decoded {
    uint16_t raw;
    uint16_t nnn;
    uint8_t nn;
    uint8_t n;
    uint8_t x;
    uint8_t y;
};

inline Decoded decode(uint16_t op) {
    return Decoded{
        op,
        uint16_t(op & 0x0FFF),
        uint8_t(op & 0x00FF),
        uint8_t(op & 0x000F),
        uint8_t((op & 0x0F00) >> 8),
        uint8_t((op & 0x00F0) >> 4)
    };
}

class Chip8 {

    public:
        Chip8(Settings settings);
        void init();
        void tickTimers();
        bool isBeeping() const;
        bool isHires() const;
        int screenWidth() const;
        int screenHeight() const;
        bool isHalted() const;
        void cycle();
        const std::array<std::array<uint8_t, 128>, 64>& getDisplayBuffer();

        bool displayBufferUpdated;
        std::array<uint8_t, 16> keypad{};

    private:
        using MemHandler = void (Chip8::*)(const Decoded&) noexcept;
        
        struct OpEntry {
            uint16_t mask;
            uint16_t value;
            MemHandler handler;
            constexpr bool match(uint16_t op) const { return (op & mask) == value; }
        };

        uint16_t PC;
        uint16_t I;
        uint16_t SP;
        std::array<uint8_t, 16> V;
        std::array<uint8_t, 8> RPL{};

        std::array<uint8_t, 4096> memory{};
        std::array<uint16_t, 16> stack{};
        std::array<std::array<uint8_t, 128>, 64> displayBuffer{};
        std::array<uint8_t, 16> prevKeypad{};
        bool hires;
        bool halted;

        uint8_t delayTimer;
        uint8_t soundTimer;

        std::mt19937 rng;
        std::uniform_int_distribution<uint8_t> randByte;
        Settings settings;

        void handleDraw(uint16_t opcode);
        void handleArithmetic(uint16_t opcode);

        void op_00E0(const Decoded& d) noexcept;
        void op_00EE(const Decoded& d) noexcept;
        void op_00FE(const Decoded& d) noexcept;
        void op_00FF(const Decoded& d) noexcept;
        void op_00CN(const Decoded& d) noexcept;
        void op_00FB(const Decoded& d) noexcept;
        void op_00FC(const Decoded& d) noexcept;
        void op_00FD(const Decoded& d) noexcept;
        void op_1nnn(const Decoded& d) noexcept;
        void op_2nnn(const Decoded& d) noexcept;
        void op_3xkk(const Decoded& d) noexcept;
        void op_4xkk(const Decoded& d) noexcept;
        void op_5xy0(const Decoded& d) noexcept;
        void op_6xkk(const Decoded& d) noexcept;
        void op_7xkk(const Decoded& d) noexcept;
        void op_8xy_(const Decoded& d) noexcept;
        void op_9xy0(const Decoded& d) noexcept;
        void op_Annn(const Decoded& d) noexcept;
        void op_Bnnn(const Decoded& d) noexcept;
        void op_Cxkk(const Decoded& d) noexcept;
        void op_Dxyn(const Decoded& d) noexcept;
        void op_Ex9E(const Decoded& d) noexcept;
        void op_ExA1(const Decoded& d) noexcept;
        void op_Fx__(const Decoded& d) noexcept;

        void op_8xy0(const Decoded& d) noexcept;
        void op_8xy1(const Decoded& d) noexcept;
        void op_8xy2(const Decoded& d) noexcept;
        void op_8xy3(const Decoded& d) noexcept;
        void op_8xy4(const Decoded& d) noexcept;
        void op_8xy5(const Decoded& d) noexcept;
        void op_8xy6(const Decoded& d) noexcept;
        void op_8xy7(const Decoded& d) noexcept;
        void op_8xyE(const Decoded& d) noexcept;

        void op_Fx29(const Decoded& d) noexcept;
        void op_Fx07(const Decoded& d) noexcept;
        void op_Fx0A(const Decoded& d) noexcept;
        void op_Fx15(const Decoded& d) noexcept;
        void op_Fx18(const Decoded& d) noexcept;
        void op_Fx1E(const Decoded& d) noexcept;
        void op_Fx30(const Decoded& d) noexcept;
        void op_Fx33(const Decoded& d) noexcept;
        void op_Fx55(const Decoded& d) noexcept;
        void op_Fx65(const Decoded& d) noexcept;
        void op_Fx75(const Decoded& d) noexcept;
        void op_Fx85(const Decoded& d) noexcept;

        void dispatch(const Decoded& d, const std::span<const OpEntry> table);

        inline static constexpr std::array<OpEntry, 24> MAIN_TABLE{{
            OpEntry{0xFFFF, 0x00E0, &Chip8::op_00E0},
            OpEntry{0xFFFF, 0x00EE, &Chip8::op_00EE},
            OpEntry{0xFFFF, 0x00FE, &Chip8::op_00FE},
            OpEntry{0xFFFF, 0x00FF, &Chip8::op_00FF},
            OpEntry{0xF0F0, 0x00C0, &Chip8::op_00CN},
            OpEntry{0xFFFF, 0x00FB, &Chip8::op_00FB},
            OpEntry{0xFFFF, 0x00FC, &Chip8::op_00FC},
            OpEntry{0xFFFF, 0x00FD, &Chip8::op_00FD},
            OpEntry{0xF000, 0x1000, &Chip8::op_1nnn},
            OpEntry{0xF000, 0x2000, &Chip8::op_2nnn},
            OpEntry{0xF000, 0x3000, &Chip8::op_3xkk},
            OpEntry{0xF000, 0x4000, &Chip8::op_4xkk},
            OpEntry{0xF00F, 0x5000, &Chip8::op_5xy0},
            OpEntry{0xF000, 0x6000, &Chip8::op_6xkk},
            OpEntry{0xF000, 0x7000, &Chip8::op_7xkk},
            OpEntry{0xF000, 0x8000, &Chip8::op_8xy_},
            OpEntry{0xF00F, 0x9000, &Chip8::op_9xy0},
            OpEntry{0xF000, 0xA000, &Chip8::op_Annn},
            OpEntry{0xF000, 0xB000, &Chip8::op_Bnnn},
            OpEntry{0xF000, 0xC000, &Chip8::op_Cxkk},
            OpEntry{0xF000, 0xD000, &Chip8::op_Dxyn},
            OpEntry{0xF0FF, 0xE09E, &Chip8::op_Ex9E},
            OpEntry{0xF0FF, 0xE0A1, &Chip8::op_ExA1},
            OpEntry{0xF000, 0xF000, &Chip8::op_Fx__},
        }};

        inline static constexpr std::array<OpEntry, 9> ARITH_TABLE{{
            OpEntry{0xF00F, 0x8000, &Chip8::op_8xy0},
            OpEntry{0xF00F, 0x8001, &Chip8::op_8xy1},
            OpEntry{0xF00F, 0x8002, &Chip8::op_8xy2},
            OpEntry{0xF00F, 0x8003, &Chip8::op_8xy3},
            OpEntry{0xF00F, 0x8004, &Chip8::op_8xy4},
            OpEntry{0xF00F, 0x8005, &Chip8::op_8xy5},
            OpEntry{0xF00F, 0x8006, &Chip8::op_8xy6},
            OpEntry{0xF00F, 0x8007, &Chip8::op_8xy7},
            OpEntry{0xF00F, 0x800E, &Chip8::op_8xyE},
        }};

        inline static constexpr std::array<OpEntry, 12> F_TABLE{{
            OpEntry{0xF0FF, 0xF029, &Chip8::op_Fx29},
            OpEntry{0xF0FF, 0xF007, &Chip8::op_Fx07},
            OpEntry{0xF0FF, 0xF00A, &Chip8::op_Fx0A},
            OpEntry{0xF0FF, 0xF015, &Chip8::op_Fx15},
            OpEntry{0xF0FF, 0xF018, &Chip8::op_Fx18},
            OpEntry{0xF0FF, 0xF01E, &Chip8::op_Fx1E},
            OpEntry{0xF0FF, 0xF030, &Chip8::op_Fx30},
            OpEntry{0xF0FF, 0xF033, &Chip8::op_Fx33},
            OpEntry{0xF0FF, 0xF055, &Chip8::op_Fx55},
            OpEntry{0xF0FF, 0xF065, &Chip8::op_Fx65},
            OpEntry{0xF0FF, 0xF075, &Chip8::op_Fx75},
            OpEntry{0xF0FF, 0xF085, &Chip8::op_Fx85},
        }};
};