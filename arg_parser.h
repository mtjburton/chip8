#pragma once

#include <optional>
#include "settings.h"

class ArgParser {
    
    public:
        static Settings parse(int argc, char* argv[]);

    private:
        static Settings defaultsForMode(Mode mode);
        static std::optional<bool> extract(const std::string& option, const std::string& arg);
};