#include "arg_parser.h"

#include <string>
#include <algorithm>

Settings ArgParser::parse(int argc, char* argv[]) {
    Mode mode = Mode::CHIP_8;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode=superchip") {
            mode = Mode::SUPER_CHIP;
            break;
        }
    }

    Settings settings = defaultsForMode(mode);

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg[0] != '-') {
            settings.rom = arg;
        }

        if (std::optional<bool>  opt = extract("--vfreset=", arg)) {
            settings.vfReset = *opt;
        }

        if (std::optional<bool>  opt = extract("--memory=", arg)) {
            settings.memory = *opt;
        }

        if (std::optional<bool>  opt = extract("--clipping=", arg)) {
            settings.clipping = *opt;
        }

        if (std::optional<bool>  opt = extract("--shift=", arg)) {
            settings.shift = *opt;
        }

        if (std::optional<bool>  opt = extract("--jump=", arg)) {
            settings.jump = *opt;
        }

        if (std::optional<bool>  opt = extract("--press=", arg)) {
            settings.press = *opt;
        }
    }

    return settings;
}

std::optional<bool> ArgParser::extract(const std::string& option, const std::string& arg) {
    if (arg.rfind(option, 0) != 0) {
        return std::nullopt;
    }

    std::string val = arg.substr(option.size());
    std::transform(val.begin(), val.end(), val.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (val == "1" || val == "true"  || val == "yes" || val == "on")  return true;
    if (val == "0" || val == "false" || val == "no"  || val == "off") return false;

    return std::nullopt;
}

Settings ArgParser::defaultsForMode(Mode mode) {
    Settings settings {
        .mode = mode,
        .clipping = true,
        .press = true,
    };

    switch (mode) {
        default: {
            std::printf("Unknown mode %d\n", mode);
            std::printf("Defaulting to chip8");
        }
        
        case Mode::CHIP_8: {
            settings.vfReset = true;
            settings.memory = true;
            settings.shift = false;
            settings.jump = false;
            break;
        }

        case Mode::SUPER_CHIP: {
            settings.vfReset = false;
            settings.memory = false;
            settings.shift = true;
            settings.jump = true;
            break;
        }
    }

    return settings;
}