# ===== Toolchain =====
CXX := clang++

# SDL flags (prefer sdl2-config, fallback to pkg-config)
SDL_CFLAGS  := $(shell sdl2-config --cflags 2>/dev/null || pkg-config --cflags sdl2)
SDL_LDFLAGS := $(shell sdl2-config --libs   2>/dev/null || pkg-config --libs sdl2)

# Build type: release (default) or debug
BUILD ?= release

# Common warnings + deps
COMMON_CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -MMD -MP $(SDL_CFLAGS)
COMMON_LDFLAGS  := $(SDL_LDFLAGS)

# Per-config flags
ifeq ($(BUILD),debug)
  CXXFLAGS := $(COMMON_CXXFLAGS) -O0 -g3 -fno-omit-frame-pointer
  LDFLAGS  := $(COMMON_LDFLAGS)
  BIN      := build/debug/chip8
  OBJDIR   := build/debug/obj
else ifeq ($(BUILD),asan)
  CXXFLAGS := $(COMMON_CXXFLAGS) -O0 -g3 -fno-omit-frame-pointer -fsanitize=address,undefined
  LDFLAGS  := $(COMMON_LDFLAGS)  -fsanitize=address,undefined
  BIN      := build/asan/chip8
  OBJDIR   := build/asan/obj
else
  # release
  CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -DNDEBUG
  LDFLAGS  := $(COMMON_LDFLAGS)
  BIN      := build/release/chip8
  OBJDIR   := build/release/obj
endif

# Sources / objects
SRC := main.cpp chip8.cpp window.cpp audio.cpp arg_parser.cpp
OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all clean run debug release asan

all: $(BIN)

# Convenience aliases
debug:  ; $(MAKE) BUILD=debug
release:; $(MAKE) BUILD=release
asan:   ; $(MAKE) BUILD=asan

# Link
$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile (with per-file deps)
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run current BUILD
run: $(BIN)
	./$(BIN)

# Clean everything
clean:
	rm -rf build

# Auto-generated dependencies
-include $(DEP)
