# Renderer Frontend

The renderer frontend is a command line tool for rendering midi files to wave
files. This is separate from the GUI program packaged by [upstream
Nuked-SC55](https://github.com/nukeykt/Nuked-SC55).

## Command line options

### `-v, --version`

Prints the version number and build configuration to stdout then exits
immediately.

### `-o <filename>`

Writes a wave file to `filename`. Cannot be combined with `--stdout`.

### `--stdout`

Writes the raw sample data to stdout. This is mostly used for testing the
emulator.

### `-f, --format s16|s32|f32`

Sets the output format.

- `s16`: signed 16-bit audio
- `s32`: signed 32-bit audio
- `f32`: 32-bit floating-point audio

### `--disable-oversampling`

Halves output frequency of the emulator. This trades audio quality for space.

### `--end cut|release`

Choose how the end of the track is handled:

- `cut` (default): stop rendering at the last MIDI event
- `release`: continue to render audio after the last MIDI event until silence.

### `-r, --reset gs|gm`

Sends a reset message to the emulator on startup. This is necessary to correct
pitch with some roms.

### `-n, --instances <count>`

Create `count` instances of the emulator. MIDI events will be routed to
emulator N where N is the MIDI event channel mod `count`. Use this to increase
effective polyphony. A `count` of 2 is enough to play most MIDIs without
dropping notes.

### `-d, --rom-directory <dir>`

Sets the directory to load roms from. If no specific romset flag is passed, the
emulator will pick one based on the filenames in `<dir>`. If this is not set,
the emulator will look for roms in these locations:

1. `<exe_dir>/../share/nuked-sc55`
2. `<exe_dir>`

`<exe_dir>` is the directory containing this executable.

### `--romset <name>`

If provided, this will set the romset to load. Otherwise, the renderer will
autodetect the romset based on what filenames it finds in the rom directory.

Run `nuked-sc55-render --help` to see a list of accepted romset names.
