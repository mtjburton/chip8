# CHIP-8 Emulator

This is a small CHIP-8 / Super-CHIP emulator I’ve been working on for fun. It’s not meant to be polished or cycle-accurate, but it runs the corex test suite and simple games (although some like space invaders needs flag overrides to behave correctly).

## Building

You’ll need SDL2 installed. On macOS/Linux, `sdl2-config` or `pkg-config` should be available once SDL2 is installed (e.g. with brew, apt, etc).

Then just:

`make`

That builds the binary into `build/chip8`.

## Running

The emulator takes a ROM path as a positional argument:

./build/chip8 path/to/rom.ch8

Controls are mapped like this:

```
1 2 3 4 => 1 2 3 C  
Q W E R => 4 5 6 D  
A S D F => 7 8 9 E  
Z X C V => A 0 B F  
```

Press `Esc` or close the window to quit.

## Options

All options are passed as flags on the command line. They’re mostly for tweaking different CHIP-8/SCHIP quirks.

- `--mode=chip8|superchip`
  Select the base mode (default is CHIP-8).

- `--vfreset=true|false`  
  Whether logical ops reset VF (quirk toggle).

- `--memory=true|false`  
  Whether Fx55/Fx65 increment I (quirk toggle).

- `--clipping=true|false`  
  Whether sprites are clipped or wrap around.

- `--shift=true|false`  
  Shift instructions use Vx or Vy (quirk toggle).

- `--jump=true|false`  
  Bnnn jumps with Vx offset (quirk toggle).

- `--press=true|false`  
  Fx0A waits for key **press** or **release**.

Most defaults follow CHIP-8 behavior unless you pass `--mode=superchip`.
