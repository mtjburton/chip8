#include "chip8.h"

#include <fstream>

Chip8::Chip8(Settings s) : rng(std::random_device{}()), randByte(0, 255) {
    displayBufferUpdated = false;
    delayTimer = 0;
    soundTimer = 0;
    PC = ROM_START;
    SP = 0;
    settings = s;
    hires = false;
    halted = false;
}

void Chip8::init() {
    std::copy(FONTSET.begin(), FONTSET.end(), memory.begin() + FONT_START);
    std::copy(BIGFONTSET.begin(), BIGFONTSET.end(), memory.begin() + BIGFONT_START);

    std::ifstream rom(settings.rom, std::ios::binary | std::ios::ate);
    if (!rom) {
        throw std::runtime_error("Unable to open ROM file");
    }

    std::streamsize size = rom.tellg();
    if (size > static_cast<std::streamsize>(MAX_ROM_SIZE)) {
        throw std::runtime_error("ROM too large");
    }

    rom.seekg(0, std::ios::beg);
    rom.read(reinterpret_cast<char*>(&memory[ROM_START]), size);
}

void Chip8::tickTimers() {
    if (delayTimer > 0) {
        delayTimer--;
    }

    if (soundTimer > 0) {
        soundTimer--;
    }
}

bool Chip8::isBeeping() const {
    return soundTimer > 0;
}

bool Chip8::isHires() const {
    return hires;
}

int Chip8::screenWidth() const {
    return hires ? 128 : 64;
}

int Chip8::screenHeight() const {
    return hires ? 64 : 32;
}

bool Chip8::isHalted() const {
    return halted;
}

const std::array<std::array<uint8_t, 128>, 64>& Chip8::getDisplayBuffer() {
    return displayBuffer;
}

void Chip8::dispatch(const Decoded& d, std::span<const OpEntry> table) {
    for (const auto& entry : table) {
        if (entry.match(d.raw)) {
            auto fn = entry.handler;
            if (!fn) {
                std::printf("Null handler for opcode %04X (mask %04X, value %04X)\n",
                            d.raw, entry.mask, entry.value);
                return;
            }
            
            (this->*fn)(d);
            return;
        }
    }
    std::printf("Unhandled opcode: %04X\n", d.raw);
}

void Chip8::cycle() {
    const uint16_t op = (memory[PC] << 8) | memory[PC + 1];
    PC += 2;
    const Decoded d = decode(op);

    dispatch(d, MAIN_TABLE);
    prevKeypad = keypad;
}

void Chip8::op_00E0(const Decoded&) noexcept {
    for (auto& row : displayBuffer) { 
        std::fill(row.begin(), row.end(), 0);
    }

    displayBufferUpdated = true;
}

void Chip8::op_00EE(const Decoded&) noexcept {
    if (SP == 0) { 
        std::printf("Stack underflow\n"); 
        abort(); 
    }

    PC = stack[--SP];
}

void Chip8::op_00FE(const Decoded& d) noexcept {
    op_00E0(d);
    hires = false;
}

void Chip8::op_00FF(const Decoded& d) noexcept {
    op_00E0(d);
    hires = true;
}

void Chip8::op_00CN(const Decoded& d) noexcept {
    const int width = screenWidth();
    const int height = screenHeight();
    int n = std::min<int>(d.n, height);
    
    if (n <= 0) {
        return;
    }

    for (int y = height - 1; y >= 0; --y) {
        const int src = y - n;
        for (int x = 0; x < width; ++x) {
            displayBuffer[y][x] = (src >= 0) ? displayBuffer[src][x] : 0;
        }
    }

    displayBufferUpdated = true;
}

void Chip8::op_00FB(const Decoded&) noexcept {
    const int width = screenWidth();
    const int height = screenHeight();
    const int s = 4;

    for (int y = 0; y < height; ++y) {
        for (int x = width - 1; x >= 0; --x) {
            const int src = x - s;
            displayBuffer[y][x] = (src >= 0) ? displayBuffer[y][src] : 0;
        }
    }
    displayBufferUpdated = true;
}

void Chip8::op_00FC(const Decoded&) noexcept {
    const int width = screenWidth();
    const int height = screenHeight();
    constexpr int s = 4;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int src = x + s;
            displayBuffer[y][x] = (src < width) ? displayBuffer[y][src] : 0;
        }
    }

    displayBufferUpdated = true;
}

void Chip8::op_00FD(const Decoded&) noexcept {
    halted = true;
}

void Chip8::op_1nnn(const Decoded& d) noexcept {  
    PC = d.nnn; 
}

void Chip8::op_2nnn(const Decoded& d) noexcept {
    if (SP >= 16) { 
        std::printf("Stack overflow\n"); 
        abort(); 
    }

    stack[SP++] = PC; 
    PC = d.nnn;
}

void Chip8::op_3xkk(const Decoded& d) noexcept {
    if (V[d.x] == d.nn) {
        PC += 2; 
    }
}

void Chip8::op_4xkk(const Decoded& d) noexcept {
    if (V[d.x] != d.nn) {
      PC += 2;   
    } 
}

void Chip8::op_5xy0(const Decoded& d) noexcept {  
    if (V[d.x] == V[d.y]) {
        PC += 2;
    }
}

void Chip8::op_6xkk(const Decoded& d) noexcept {  
    V[d.x] = d.nn; 
}

void Chip8::op_7xkk(const Decoded& d) noexcept {
    V[d.x] = uint8_t(V[d.x] + d.nn); 
}

void Chip8::op_8xy_(const Decoded& d) noexcept {
    dispatch(d, ARITH_TABLE);
}

void Chip8::op_9xy0(const Decoded& d) noexcept {
    if (V[d.x] != V[d.y]) {
        PC += 2;
    }
}

void Chip8::op_Annn(const Decoded& d) noexcept {  
    I = d.nnn; 
}

void Chip8::op_Bnnn(const Decoded& d) noexcept {
    uint8_t x = settings.jump ? d.x : 0;
    PC = d.nnn + V[x];
}

void Chip8::op_Cxkk(const Decoded& d) noexcept {  
    V[d.x] = uint8_t(randByte(rng) & d.nn); 
}

void Chip8::op_Dxyn(const Decoded& d) noexcept {
    const int width = screenWidth();
    const int height = screenHeight();

    uint8_t x = V[d.x] % width;
    uint8_t y = V[d.y] % height;

    const bool big = (hires && d.n == 0);
    const int spriteWidth = big ? 16 : 8;
    const int spriteHeight = big ? 16 : d.n;
    const int bytesPerRow = (spriteWidth + 7) / 8; // 1 for 8-wide, 2 for 16-wide

    V[0xF] = 0;

    for (int8_t i = 0; i < spriteHeight; i++) {
        const int memRowBase = I + i * bytesPerRow;

        for (int col = 0; col < spriteWidth; ++col) {
            const int byteIdx = col >> 3;
            const int bitIdx  = 7 - (col & 7); 
            const uint8_t byte = memory[memRowBase + byteIdx];
            const uint8_t spritePixel = (byte >> bitIdx) & 0x1;

            int xCoord = x + col;
            int yCoord = y + i;

            if (settings.clipping) {
                if (xCoord < 0 || yCoord < 0 || xCoord >= width || yCoord >= height) {
                    continue;
                }
            }

            xCoord %= width;
            yCoord %= height;

            uint8_t& screenPixel = displayBuffer[yCoord][xCoord];

            if (spritePixel == 1 && screenPixel == 1) {
                V[0xF] = 1;
            }

            screenPixel ^= spritePixel;
        }
    }

    displayBufferUpdated = true;
}

void Chip8::op_Ex9E(const Decoded& d) noexcept {
    if (keypad[V[d.x]] == 1) {
        PC += 2;
    }
}

void Chip8::op_ExA1(const Decoded& d) noexcept {
    if (keypad[V[d.x]] == 0) {
        PC += 2;
    }
}

void Chip8::op_Fx__(const Decoded& d) noexcept {
    dispatch(d, F_TABLE);
}

void Chip8::op_8xy0(const Decoded& d) noexcept {  
    V[d.x] = V[d.y]; 
}

void Chip8::op_8xy1(const Decoded& d) noexcept {
    V[d.x] = V[d.x] | V[d.y];
    if (settings.vfReset) {
        V[0xF] = 0;
    }
}

void Chip8::op_8xy2(const Decoded& d) noexcept {
    V[d.x] = V[d.x] & V[d.y];
    if (settings.vfReset) { 
        V[0xF] = 0;
    }
}

void Chip8::op_8xy3(const Decoded& d) noexcept {
    V[d.x] = V[d.x] ^ V[d.y];
    if (settings.vfReset) { 
        V[0xF] = 0;
    }
}

void Chip8::op_8xy4(const Decoded& d) noexcept {
    uint16_t r = uint16_t(V[d.x]) + uint16_t(V[d.y]);
    V[d.x] = uint8_t(r & 0xFF);
    V[0xF] = (r > 0xFF) ? 1 : 0;
}

void Chip8::op_8xy5(const Decoded& d) noexcept {
    uint8_t x = V[d.x], y = V[d.y];
    V[d.x] = uint8_t(x - y);
    V[0xF] = (x >= y) ? 1 : 0;
}

void Chip8::op_8xy6(const Decoded& d) noexcept {
    if (!settings.shift) { 
        V[d.x] = V[d.y];
    }

    uint8_t bit = V[d.x] & 0x1;
    V[d.x] >>= 1;
    V[0xF] = bit;
}

void Chip8::op_8xy7(const Decoded& d) noexcept {
    uint8_t x = V[d.x], y = V[d.y];
    V[d.x] = uint8_t(y - x);
    V[0xF] = (y >= x) ? 1 : 0;
}

void Chip8::op_8xyE(const Decoded& d) noexcept {
    if (!settings.shift) { 
        V[d.x] = V[d.y];
    }
    
    uint8_t bit = (V[d.x] & 0x80) >> 7;
    V[d.x] <<= 1;
    V[0xF] = bit;
}

void Chip8::op_Fx29(const Decoded& d) noexcept {
    I = FONT_START + (V[d.x] * 5); 
}

void Chip8::op_Fx07(const Decoded& d) noexcept { 
    V[d.x] = delayTimer; 
}

void Chip8::op_Fx0A(const Decoded& d) noexcept {
    bool keyPressed = false;

    for (uint8_t i = 0; i < 16; ++i) {
        if ((!settings.press && (prevKeypad[i] == 1 && keypad[i] == 0))
         || ( settings.press && (prevKeypad[i] == 0 && keypad[i] == 1))) {
            V[d.x] = i; keyPressed = true; break;
        }
    }

    if (!keyPressed) { 
        PC -= 2;
    }
}

void Chip8::op_Fx15(const Decoded& d) noexcept {
    delayTimer = V[d.x]; 
}

void Chip8::op_Fx18(const Decoded& d) noexcept {
    soundTimer = V[d.x]; 
}

void Chip8::op_Fx1E(const Decoded& d) noexcept {
    I += V[d.x]; 
}

void Chip8::op_Fx30(const Decoded& d) noexcept {
    I = BIGFONT_START + (V[d.x] & 0x0F) * 10;
}

void Chip8::op_Fx33(const Decoded& d) noexcept {
    uint8_t n = V[d.x];
    memory[I]   = n / 100;
    memory[I+1] = (n / 10) % 10;
    memory[I+2] = n % 10;
}

void Chip8::op_Fx55(const Decoded& d) noexcept {
    for (uint16_t i = 0; i <= d.x; ++i) {
        memory[I + i] = V[i];
    }

    if (settings.memory) {
        I += (d.x + 1);
    }
}

void Chip8::op_Fx65(const Decoded& d) noexcept {
    for (uint16_t i = 0; i <= d.x; ++i) {
        V[i] = memory[I + i];
    }
    
    if (settings.memory) {
        I += (d.x + 1);
    }
}

void Chip8::op_Fx75(const Decoded& d) noexcept {
    uint8_t n = std::min<uint8_t>(d.x + 1, 8);

    for (uint8_t i = 0; i < n; ++i) {
        RPL[i] = V[i];
    }
}
void Chip8::op_Fx85(const Decoded& d) noexcept {
    uint8_t n = std::min<uint8_t>(d.x + 1, 8);

    for (uint8_t i = 0; i < n; ++i) {
        V[i] = RPL[i];
    }
}