#pragma once

#include <string>

enum Mode {
    CHIP_8,
    SUPER_CHIP,
};

struct Settings {
    Mode mode;
    std::string rom;
    
    bool vfReset;
    bool memory;
    bool clipping;
    bool shift;
    bool jump;
    bool press;
};