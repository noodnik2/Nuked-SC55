# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A noodnik2 fork of [jcmoyer/Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55) — a Roland SC-55 MIDI synthesizer emulator. C++23, CMake build, two executable frontends sharing a common backend library. ROMs from real hardware are required at runtime (not included).

## Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

Optimized native build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-march=native -mtune=native" \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
cmake --build .
```

**macOS**: requires `brew install rtmidi`. SDL2 optional (standard frontend won't build without it).  
**Windows**: pass `-DCMAKE_PREFIX_PATH=<SDL2_cmake_dir>` for SDL2; add `-DNUKED_ENABLE_ASIO=ON -DNUKED_ASIO_SDK_DIR=<path>` for ASIO support.  
**Linux**: rtmidi required.

Key CMake options:
- `NUKED_ENABLE_TESTS=ON` — enable test suite (also set `NUKED_TEST_ROMDIR=<path>`)
- `NUKED_ENABLE_ASIO=ON` — Windows-only ASIO audio output

## Tests

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DNUKED_ENABLE_TESTS=ON -DNUKED_TEST_ROMDIR=<rom_path> ..
cmake --build .
ctest . -C Release
ctest . -C Release -j<num_jobs>   # parallel
```

Requires **Catch2 v3.7.0** in `CMAKE_PREFIX_PATH`.

Integration tests (`test/integration/`) invoke `nuked-sc55-render --stdout`, pipe raw audio through SHA256, and compare against pinned hashes — one test per MIDI file per romset (mk1 + mk2). The 32 MIDI files are from the Alien Vendetta MIDI Pack (`avmidi/`). Unit tests (`test/test_ringbuffer.cpp`) cover the ringbuffer.

## Architecture

### Backend / frontend split

```
src/backend/     → library: nuked-sc55-backend
src/standard/    → executable: nuked-sc55      (interactive, SDL2 + MIDI input)
src/renderer/    → executable: nuked-sc55-render (batch MIDI-to-WAV, no GUI)
```

`nuked-sc55-backend` is pure emulation with no UI dependencies. Third-party frontends can link against it directly.

### Backend internals

- **`Emulator`** (`emu.h/cpp`) — top-level orchestrator; owns `mcu_t`, `submcu_t`, `pcm_t`, `lcd_t`, `mcu_timer_t`. Exposes `Init`, `Reset`, `Step`, `PostMIDI`, `PostSystemReset`. `Step()` delegates directly to `MCU_Step()`.
- **`mcu_t`** (`mcu.h/cpp`) — Hitachi H8/300 CPU: 8 general registers, PC, SR, ROM/RAM/SRAM banks, interrupt vectors, opcode dispatch table (256 entries). `MCU_Operand_Read()` and `MCU_ReadCodeAdvance()` are the most-connected functions in the codebase (30 and 21 edges respectively) — nearly every opcode touches them.
- **`submcu_t`** (`submcu.h/cpp`) — secondary MCU (absent in some romsets, e.g. SCB-55). Its `SM_ReadAdvance()` is similarly central (28 edges).
- **`pcm_t`** (`pcm.h/cpp`) — audio synthesis; `PCM_Update → PCM_ReadROM → MCU_Interrupt_SetRequest(IRQ0)` is the core voice-rendering loop.
- **`lcd_t`** (`lcd.h/cpp`) — display driver; renders via `LCD_Backend` callback so frontends can show/hide the LCD window independently. LCD is memory-mapped: writes to MCU address space route directly through `MCU_Write() → LCD_Write()`.
- **`mcu_timer_t`** (`mcu_timer.h/cpp`) — timer peripherals feeding interrupt requests.

Audio output from the backend is a stream of stereo `AudioFrame` samples (templated on `int16_t`, `int32_t`, or `float`). The frontend registers a callback (`MCU_PostSample`) to receive them.

`PCM_GetOutputFrequency()` is the highest-betweenness bridge in the graph — it ties Config/SIMD math, audio output backends, hardware init, and frontend entry points together. Any change to output frequency cascades through all of these.

Supported romsets: `MK2`, `ST`, `MK1`, `CM300`, `JV880`, `SCB55`, `RLP3237`, `SC155`, `SC155MK2`.

### Standard frontend (`src/standard/`)

Interactive use: SDL2 window with LCD display, MIDI input (rtmidi or Win32 native), audio output (SDL or ASIO). Multiple emulator instances (`-n`) distribute MIDI channels across instances for increased polyphony — `FE_RouteMIDI` sends SysEx as broadcast to all instances and routes channel messages by `channel % instances_in_use`. Key files: `main.cpp` (FE_Application / FE_Instance lifecycle: `main → FE_CreateInstance → FE_Run → FE_Quit`), `audio_sdl.cpp`, `lcd_sdl.cpp`, `midi_rtmidi.cpp` / `midi_win32.cpp`, `output_sdl.cpp` / `output_asio.cpp`.

SDL audio output uses a `RingbufferView` in a callback pattern (`AudioCallback → SDLOutput → RingbufferView`). ASIO output uses a global `ASIOOutput` struct — this is intentional: ASIO callbacks don't carry a userdata pointer so there's no way to avoid process-global state.

### Renderer frontend (`src/renderer/`)

Batch MIDI-to-WAV with no GUI. Reads `.mid` via `smf.cpp` (`SMF_LoadEvents → SMF_MergeTracks`), renders through the emulator, writes WAV via `wav.cpp`. `R_RenderTrack` drives the pipeline; `R_Mixer` and `R_ChunkQueue` handle multi-instance mixing. WAV INFO chunk carries metadata extracted from MIDI meta-events (title, artist, etc.) via `WavMetadata` — the metadata path is `processMeta → SMF_Data → WavMetadata → writeInfoChunk`. `R_RenderTrack` also calls `EMU_ParseRomsetName()` to embed the romset name in the output filename.

## Contribution policy

Backend changes must be submitted upstream to `nukeykt/Nuked-SC55` first; this fork cherry-picks them back. Frontend and tooling changes unique to this fork do not require upstream submission. See `CONTRIBUTING.md`.

## ROM directory search order

If `-d` is not passed, the binary looks for ROMs in:
1. `<exe_dir>/../share/nuked-sc55`
2. `<exe_dir>`

Romset is auto-detected from filenames if `--romset` is not specified.

## Knowledge Graph

This project has been analyzed with graphify. Output is in `graphify-out/`.

Before answering questions about architecture, history, themes, or
relationships in this codebase, consult the knowledge graph:

- Read `graphify-out/GRAPH_REPORT.md` for a structural overview,
  god nodes, and community summaries
- For specific entity lookups, use `graphify-out/graph.json`
- The graph covers: backend emulation (MCU/SubMCU opcodes, PCM voice pipeline, LCD, interrupts), both frontends (standard SDL+ASIO, renderer), build system, and documentation

Confidence tags in the graph:
- EXTRACTED = directly stated in source
- INFERRED = implied, check confidence score before relying on it
