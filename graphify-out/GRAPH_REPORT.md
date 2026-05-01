# Graph Report - .  (2026-04-30)

## Corpus Check
- 57 files · ~247,861 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 628 nodes · 1061 edges · 57 communities detected
- Extraction: 80% EXTRACTED · 20% INFERRED · 0% AMBIGUOUS · INFERRED: 216 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_MCU Core & Interrupts|MCU Core & Interrupts]]
- [[_COMMUNITY_Hardware Init & Reset|Hardware Init & Reset]]
- [[_COMMUNITY_SubMCU Execution Engine|SubMCU Execution Engine]]
- [[_COMMUNITY_Frontend Entry Points|Frontend Entry Points]]
- [[_COMMUNITY_Config & SIMD Math|Config & SIMD Math]]
- [[_COMMUNITY_SDL Frontend Layer|SDL Frontend Layer]]
- [[_COMMUNITY_Emulator Options & Audio Types|Emulator Options & Audio Types]]
- [[_COMMUNITY_ROM Set Management|ROM Set Management]]
- [[_COMMUNITY_Audio Output Backends|Audio Output Backends]]
- [[_COMMUNITY_Renderer Core|Renderer Core]]
- [[_COMMUNITY_MIDI File Parsing|MIDI File Parsing]]
- [[_COMMUNITY_WAV File IO|WAV File I/O]]
- [[_COMMUNITY_Build System & Tests|Build System & Tests]]
- [[_COMMUNITY_Application Lifecycle|Application Lifecycle]]
- [[_COMMUNITY_MCU Opcode Handlers|MCU Opcode Handlers]]
- [[_COMMUNITY_Ring Buffer|Ring Buffer]]
- [[_COMMUNITY_LCD Font Rendering|LCD Font Rendering]]
- [[_COMMUNITY_MIDI Routing|MIDI Routing]]
- [[_COMMUNITY_Saturation Math Utilities|Saturation Math Utilities]]
- [[_COMMUNITY_WAV Metadata Writing|WAV Metadata Writing]]
- [[_COMMUNITY_PCM Config & Write|PCM Config & Write]]
- [[_COMMUNITY_LCD Backlight Display|LCD Backlight Display]]
- [[_COMMUNITY_Audio Output Kinds|Audio Output Kinds]]
- [[_COMMUNITY_MIDI Quit Impls|MIDI Quit Impls]]
- [[_COMMUNITY_Project Overview|Project Overview]]
- [[_COMMUNITY_ASIO Build & Changelog|ASIO Build & Changelog]]
- [[_COMMUNITY_RingbufferView Test|RingbufferView Test]]
- [[_COMMUNITY_GenericBuffer|GenericBuffer]]
- [[_COMMUNITY_Renderer Parameters|Renderer Parameters]]
- [[_COMMUNITY_Renderer Parse Error|Renderer Parse Error]]
- [[_COMMUNITY_Renderer End Behavior|Renderer End Behavior]]
- [[_COMMUNITY_LCD Write|LCD Write]]
- [[_COMMUNITY_Timer Clock|Timer Clock]]
- [[_COMMUNITY_MCU Step|MCU Step]]
- [[_COMMUNITY_MCU Init|MCU Init]]
- [[_COMMUNITY_MCU Post UART|MCU Post UART]]
- [[_COMMUNITY_MCU Post Sample|MCU Post Sample]]
- [[_COMMUNITY_Romset Enum|Romset Enum]]
- [[_COMMUNITY_Audio Normalize|Audio Normalize]]
- [[_COMMUNITY_EMU Romset Name|EMU Romset Name]]
- [[_COMMUNITY_EMU Parse Romset|EMU Parse Romset]]
- [[_COMMUNITY_EMU Get Romset Names|EMU Get Romset Names]]
- [[_COMMUNITY_PCM Output Frequency|PCM Output Frequency]]
- [[_COMMUNITY_SubMCU Sys Write|SubMCU Sys Write]]
- [[_COMMUNITY_SubMCU Sys Read|SubMCU Sys Read]]
- [[_COMMUNITY_SubMCU Post UART|SubMCU Post UART]]
- [[_COMMUNITY_IRQ0 Interrupt Source|IRQ0 Interrupt Source]]
- [[_COMMUNITY_MCU Jump RTS|MCU Jump RTS]]
- [[_COMMUNITY_Horizontal Add F32|Horizontal Add F32]]
- [[_COMMUNITY_Config Version Writer|Config Version Writer]]
- [[_COMMUNITY_MIDI Init|MIDI Init]]
- [[_COMMUNITY_MIDI Quit|MIDI Quit]]
- [[_COMMUNITY_MIDI Print Devices|MIDI Print Devices]]
- [[_COMMUNITY_MIDI Print Devices Win32|MIDI Print Devices Win32]]
- [[_COMMUNITY_ASIO Output Struct|ASIO Output Struct]]
- [[_COMMUNITY_Frontend Parameters|Frontend Parameters]]
- [[_COMMUNITY_Frontend Parse Error|Frontend Parse Error]]

## God Nodes (most connected - your core abstractions)
1. `MCU_Operand_Read()` - 30 edges
2. `SM_ReadAdvance()` - 28 edges
3. `MCU_ReadCodeAdvance()` - 21 edges
4. `MCU_ErrorTrap()` - 20 edges
5. `SM_Read()` - 18 edges
6. `SM_ReadAdvance16()` - 17 edges
7. `SM_Update_NZ()` - 17 edges
8. `MCU_SetStatusCommon()` - 17 edges
9. `MCU_SetStatus()` - 16 edges
10. `MCU_Operand_Write()` - 16 edges

## Surprising Connections (you probably didn't know these)
- `Integration Test Runner main` --conceptually_related_to--> `WAV_Handle`  [INFERRED]
  test/integration/test_runner.py → src/renderer/wav.h
- `R_RenderTrack()` --calls--> `EMU_ParseRomsetName()`  [INFERRED]
  src/renderer/main.cpp → src/backend/emu.cpp
- `MCU_Write()` --calls--> `LCD_Write()`  [INFERRED]
  src/backend/mcu.cpp → src/backend/lcd.cpp
- `Step()` --calls--> `MCU_Step()`  [INFERRED]
  src/backend/emu.cpp → src/backend/mcu.cpp
- `Renderer Frontend Documentation` --references--> `nuked-sc55-render (CMake executable target)`  [INFERRED]
  doc/renderer_frontend.md → CMakeLists.txt

## Hyperedges (group relationships)
- **MIDI-to-WAV Render Pipeline** — smf_smf_loadevents, main_r_rendertrack, wav_wav_handle [INFERRED 0.90]
- **SMF Metadata Extraction and WAV Embedding** — smf_processmeta, smf_smf_data, wav_wavmetadata [INFERRED 0.90]
- **MCU/SubMCU Interrupt Handling System** — mcu_mcu_t, submcu_submcu_t, mcu_interrupt_mcu_interrupt_handle [INFERRED 0.85]
- **Emulator Subsystem Initialization Flow** — emu_emulator, pcm_pcm_init, submcu_sm_init [EXTRACTED 0.95]
- **PCM WaveROM Voice Rendering Pipeline** — pcm_pcm_update, pcm_pcm_readrom, mcu_interrupt_mcu_interrupt_setrequest [EXTRACTED 0.90]
- **SDL Audio Output via RingbufferView Pipeline** — output_sdl_audiocallback, output_sdl_sdloutput, ringbuffer_ringbufferview [EXTRACTED 0.95]
- **Platform-Agnostic MIDI Abstraction Layer** — midi_win32_midi_init, midi_rtmidi_midi_init, main_fe_routemidi [INFERRED 0.90]
- **Audio Output Abstraction (SDL + ASIO)** — output_sdl_out_sdl_create, output_asio_out_asio_create, main_fe_openaudio [INFERRED 0.90]
- **Standard Frontend Application Lifecycle** — main_main, main_fe_createinstance, main_fe_run, main_fe_quit [EXTRACTED 1.00]

## Communities

### Community 0 - "MCU Core & Interrupts"
Cohesion: 0.09
Nodes (63): MCU_Interrupt_Exception(), MCU_ControlRegisterRead(), MCU_ControlRegisterWrite(), MCU_ErrorTrap(), MCU_GetAddress(), MCU_GetPageForRegister(), MCU_PopStack(), MCU_PushStack() (+55 more)

### Community 1 - "Hardware Init & Reset"
Cohesion: 0.06
Nodes (59): Init(), Reset(), LCD_Init(), MCU_Interrupt_Handle(), MCU_Interrupt_SetRequest(), MCU_Interrupt_Start(), MCU_Interrupt_StartVector(), MCU_Interrupt_TRAPA() (+51 more)

### Community 2 - "SubMCU Execution Engine"
Cohesion: 0.1
Nodes (56): SM_ErrorTrap(), SM_GetVectorAddress(), SM_HandleInterrupt(), SM_Opcode_AND(), SM_Opcode_BBC_BBS(), SM_Opcode_BCC(), SM_Opcode_BCS(), SM_Opcode_BEQ() (+48 more)

### Community 3 - "Frontend Entry Points"
Cohesion: 0.06
Nodes (41): EMU_DetectRomset(), P_GetProcessPath(), FE_AllocateInstance(), FE_BroadcastMIDI(), FE_CreateInstance(), FE_DestroyInstance(), FE_EventLoop(), FE_FixupParameters() (+33 more)

### Community 4 - "Config & SIMD Math"
Cohesion: 0.07
Nodes (32): Cfg_WriteVersionInfo(), HorizontalAddF32(), HorizontalSatAddI16(), HorizontalSatAddI32(), SaturatingAdd(), PCM_GetOutputFrequency(), AllocChunk(), joinWithSpaces() (+24 more)

### Community 5 - "SDL Frontend Layer"
Cohesion: 0.06
Nodes (39): AudioFormatToSDLAudioFormat Function, SDLAudioFormatToString Function, RangeCast Template Function, CommandLineReader, LCD_SDL_Backend (class), LCD_SDL_Backend::HandleEvent, LCD_SDL_Backend::Render, LCD_SDL_Backend::Start (+31 more)

### Community 6 - "Emulator Options & Audio Types"
Cohesion: 0.06
Nodes (34): AudioFrame template, EMU_DetectRomset Function, EMU_Options Struct, EMU_ReadStreamExact Function, EMU_ReadStreamUpTo Function, EMU_SystemReset Enum, Emulator Class, unscramble Function (+26 more)

### Community 7 - "ROM Set Management"
Cohesion: 0.08
Nodes (24): EMU_GetParsableRomsetNames(), EMU_ParseRomsetName(), EMU_ReadStreamExact(), EMU_ReadStreamUpTo(), EMU_RomsetName(), LoadRoms(), PostMIDI(), PostSystemReset() (+16 more)

### Community 8 - "Audio Output Backends"
Cohesion: 0.14
Nodes (22): AudioFormatToSDLAudioFormat(), SDLAudioFormatToString(), FE_OpenASIOAudio(), FE_RunInstanceASIO(), bufferSwitch(), bufferSwitchTimeInfo(), Deinterleave(), Deinterleave16() (+14 more)

### Community 9 - "Renderer Core"
Cohesion: 0.1
Nodes (25): AudioFormat, R_ChunkQueue, R_FrameChunk, R_Mixer, R_MixOut, R_MixOutState, R_OwnedChunk, R_RenderOne (+17 more)

### Community 10 - "MIDI File Parsing"
Cohesion: 0.2
Nodes (13): addToVector(), Check(), processMeta(), SMF_IsStatusByte(), SMF_LoadEvents(), SMF_MergeTracks(), SMF_ReadAllBytes(), SMF_ReadChunk() (+5 more)

### Community 11 - "WAV File I/O"
Cohesion: 0.24
Nodes (11): Close(), Finish(), ~WAV_Handle(), WAV_WriteBytes(), WAV_WriteCString(), WAV_WriteF32LE(), WAV_WriteU16LE(), WAV_WriteU32LE() (+3 more)

### Community 12 - "Build System & Tests"
Cohesion: 0.14
Nodes (14): Alien Vendetta MIDI Pack (AVmidi), Integration Test Suite, Version 0.6.0 (TBD), asio_sdk (CMake library target), NUKED_ENABLE_ASIO (CMake option), nuked-sc55 (CMake executable target), nuked-sc55-backend (CMake library target), nuked-sc55-render (CMake executable target) (+6 more)

### Community 13 - "Application Lifecycle"
Cohesion: 0.18
Nodes (12): FE_Application (struct), FE_Instance (struct), FE_ReceiveSampleASIO, FE_ReceiveSampleSDL, FE_RunInstanceASIO, FE_RunInstanceSDL, MIDI_Init (RtMidi), MIDI_PickInputDevice (RtMidi) (+4 more)

### Community 14 - "MCU Opcode Handlers"
Cohesion: 0.18
Nodes (11): MCU_Interrupt_Exception, MCU_Interrupt_TRAPA Function, MCU_ADD_Common Function, MCU_Jump_Bcc Function, MCU_Jump_JMP Function, MCU_Jump_JSR Function, MCU_Opcode_Table Dispatch Table, MCU_Operand_General Function (+3 more)

### Community 16 - "Ring Buffer"
Cohesion: 0.33
Nodes (4): GetReadPtr(), Mask(), RingbufferView(), CreateAndPrepareBuffer()

### Community 18 - "LCD Font Rendering"
Cohesion: 0.6
Nodes (5): lcd_font (bitmap data), LCD_FontRenderLevel, LCD_FontRenderLR, LCD_FontRenderStandard, LCD_Render

### Community 19 - "MIDI Routing"
Cohesion: 0.5
Nodes (5): FE_BroadcastMIDI, FE_RouteMIDI, FE_SendMIDI, MidiOnReceive, MIDI_Callback (Win32)

### Community 21 - "Saturation Math Utilities"
Cohesion: 0.5
Nodes (4): Clamp Template Function, HorizontalSatAddI16 Function, HorizontalSatAddI32 Function, SaturatingAdd Function

### Community 22 - "WAV Metadata Writing"
Cohesion: 0.67
Nodes (3): WAV_Handle::Finish, writeInfoChunk, writeMetaTag

### Community 28 - "PCM Config & Write"
Cohesion: 1.0
Nodes (2): PCM_GetConfig Function, PCM_Write Function

### Community 29 - "LCD Backlight Display"
Cohesion: 1.0
Nodes (2): LCD Back Pixel Data, LCD Back Palette Data

### Community 30 - "Audio Output Kinds"
Cohesion: 1.0
Nodes (2): AudioOutputKind Enum, AudioOutputList Type

### Community 31 - "MIDI Quit Impls"
Cohesion: 1.0
Nodes (2): MIDI_Quit (RtMidi), MIDI_Quit (Win32)

### Community 32 - "Project Overview"
Cohesion: 1.0
Nodes (2): noodnik2 Fork Notes, Nuked SC-55 Project

### Community 33 - "ASIO Build & Changelog"
Cohesion: 1.0
Nodes (2): ASIO Support Build Instructions, Version 0.5.0 (2025-03-21)

### Community 46 - "RingbufferView Test"
Cohesion: 1.0
Nodes (1): RingbufferView Test

### Community 47 - "GenericBuffer"
Cohesion: 1.0
Nodes (1): GenericBuffer

### Community 48 - "Renderer Parameters"
Cohesion: 1.0
Nodes (1): R_Parameters

### Community 49 - "Renderer Parse Error"
Cohesion: 1.0
Nodes (1): R_ParseError

### Community 50 - "Renderer End Behavior"
Cohesion: 1.0
Nodes (1): R_EndBehavior

### Community 51 - "LCD Write"
Cohesion: 1.0
Nodes (1): LCD_Write

### Community 52 - "Timer Clock"
Cohesion: 1.0
Nodes (1): TIMER_Clock

### Community 53 - "MCU Step"
Cohesion: 1.0
Nodes (1): MCU_Step

### Community 54 - "MCU Init"
Cohesion: 1.0
Nodes (1): MCU_Init

### Community 55 - "MCU Post UART"
Cohesion: 1.0
Nodes (1): MCU_PostUART

### Community 56 - "MCU Post Sample"
Cohesion: 1.0
Nodes (1): MCU_PostSample

### Community 57 - "Romset Enum"
Cohesion: 1.0
Nodes (1): Romset enum

### Community 58 - "Audio Normalize"
Cohesion: 1.0
Nodes (1): Normalize (AudioFrame conversion)

### Community 59 - "EMU Romset Name"
Cohesion: 1.0
Nodes (1): EMU_RomsetName Function

### Community 60 - "EMU Parse Romset"
Cohesion: 1.0
Nodes (1): EMU_ParseRomsetName Function

### Community 61 - "EMU Get Romset Names"
Cohesion: 1.0
Nodes (1): EMU_GetParsableRomsetNames Function

### Community 62 - "PCM Output Frequency"
Cohesion: 1.0
Nodes (1): PCM_GetOutputFrequency Function

### Community 63 - "SubMCU Sys Write"
Cohesion: 1.0
Nodes (1): SM_SysWrite Function

### Community 64 - "SubMCU Sys Read"
Cohesion: 1.0
Nodes (1): SM_SysRead Function

### Community 65 - "SubMCU Post UART"
Cohesion: 1.0
Nodes (1): SM_PostUART Function

### Community 66 - "IRQ0 Interrupt Source"
Cohesion: 1.0
Nodes (1): INTERRUPT_SOURCE_IRQ0 (GPINT)

### Community 67 - "MCU Jump RTS"
Cohesion: 1.0
Nodes (1): MCU_Jump_RTS Function

### Community 68 - "Horizontal Add F32"
Cohesion: 1.0
Nodes (1): HorizontalAddF32 Function

### Community 69 - "Config Version Writer"
Cohesion: 1.0
Nodes (1): Cfg_WriteVersionInfo Function

### Community 70 - "MIDI Init"
Cohesion: 1.0
Nodes (1): MIDI_Init Function

### Community 71 - "MIDI Quit"
Cohesion: 1.0
Nodes (1): MIDI_Quit Function

### Community 72 - "MIDI Print Devices"
Cohesion: 1.0
Nodes (1): MIDI_PrintDevices Function

### Community 73 - "MIDI Print Devices Win32"
Cohesion: 1.0
Nodes (1): MIDI_PrintDevices (Win32)

### Community 74 - "ASIO Output Struct"
Cohesion: 1.0
Nodes (1): ASIOOutput (struct)

### Community 75 - "Frontend Parameters"
Cohesion: 1.0
Nodes (1): FE_Parameters (struct)

### Community 76 - "Frontend Parse Error"
Cohesion: 1.0
Nodes (1): FE_ParseError (enum)

## Knowledge Gaps
- **111 isolated node(s):** `R_FrameChunk`, `R_Mixer`, `RingbufferView Test`, `GenericBuffer`, `Integration Test Runner main` (+106 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `PCM Config & Write`** (2 nodes): `PCM_GetConfig Function`, `PCM_Write Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `LCD Backlight Display`** (2 nodes): `LCD Back Pixel Data`, `LCD Back Palette Data`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Audio Output Kinds`** (2 nodes): `AudioOutputKind Enum`, `AudioOutputList Type`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MIDI Quit Impls`** (2 nodes): `MIDI_Quit (RtMidi)`, `MIDI_Quit (Win32)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Project Overview`** (2 nodes): `noodnik2 Fork Notes`, `Nuked SC-55 Project`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `ASIO Build & Changelog`** (2 nodes): `ASIO Support Build Instructions`, `Version 0.5.0 (2025-03-21)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `RingbufferView Test`** (1 nodes): `RingbufferView Test`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `GenericBuffer`** (1 nodes): `GenericBuffer`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Renderer Parameters`** (1 nodes): `R_Parameters`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Renderer Parse Error`** (1 nodes): `R_ParseError`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Renderer End Behavior`** (1 nodes): `R_EndBehavior`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `LCD Write`** (1 nodes): `LCD_Write`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Timer Clock`** (1 nodes): `TIMER_Clock`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MCU Step`** (1 nodes): `MCU_Step`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MCU Init`** (1 nodes): `MCU_Init`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MCU Post UART`** (1 nodes): `MCU_PostUART`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MCU Post Sample`** (1 nodes): `MCU_PostSample`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Romset Enum`** (1 nodes): `Romset enum`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Audio Normalize`** (1 nodes): `Normalize (AudioFrame conversion)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `EMU Romset Name`** (1 nodes): `EMU_RomsetName Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `EMU Parse Romset`** (1 nodes): `EMU_ParseRomsetName Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `EMU Get Romset Names`** (1 nodes): `EMU_GetParsableRomsetNames Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `PCM Output Frequency`** (1 nodes): `PCM_GetOutputFrequency Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `SubMCU Sys Write`** (1 nodes): `SM_SysWrite Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `SubMCU Sys Read`** (1 nodes): `SM_SysRead Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `SubMCU Post UART`** (1 nodes): `SM_PostUART Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `IRQ0 Interrupt Source`** (1 nodes): `INTERRUPT_SOURCE_IRQ0 (GPINT)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MCU Jump RTS`** (1 nodes): `MCU_Jump_RTS Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Horizontal Add F32`** (1 nodes): `HorizontalAddF32 Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Config Version Writer`** (1 nodes): `Cfg_WriteVersionInfo Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MIDI Init`** (1 nodes): `MIDI_Init Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MIDI Quit`** (1 nodes): `MIDI_Quit Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MIDI Print Devices`** (1 nodes): `MIDI_PrintDevices Function`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `MIDI Print Devices Win32`** (1 nodes): `MIDI_PrintDevices (Win32)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `ASIO Output Struct`** (1 nodes): `ASIOOutput (struct)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Frontend Parameters`** (1 nodes): `FE_Parameters (struct)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Frontend Parse Error`** (1 nodes): `FE_ParseError (enum)`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `PCM_GetOutputFrequency()` connect `Config & SIMD Math` to `Audio Output Backends`, `Hardware Init & Reset`, `Frontend Entry Points`?**
  _High betweenness centrality (0.096) - this node is a cross-community bridge._
- **Why does `R_RenderTrack()` connect `Config & SIMD Math` to `MIDI File Parsing`, `Frontend Entry Points`, `ROM Set Management`?**
  _High betweenness centrality (0.062) - this node is a cross-community bridge._
- **Why does `MCU_Read()` connect `Hardware Init & Reset` to `MCU Core & Interrupts`, `ROM Set Management`?**
  _High betweenness centrality (0.059) - this node is a cross-community bridge._
- **Are the 4 inferred relationships involving `MCU_Operand_Read()` (e.g. with `MCU_Interrupt_Exception()` and `MCU_Read16()`) actually correct?**
  _`MCU_Operand_Read()` has 4 INFERRED edges - model-reasoned connections that need verification._
- **Are the 19 inferred relationships involving `MCU_ReadCodeAdvance()` (e.g. with `MCU_ReadInstruction()` and `MCU_LDM()`) actually correct?**
  _`MCU_ReadCodeAdvance()` has 19 INFERRED edges - model-reasoned connections that need verification._
- **Are the 19 inferred relationships involving `MCU_ErrorTrap()` (e.g. with `MCU_ControlRegisterWrite()` and `MCU_ControlRegisterRead()`) actually correct?**
  _`MCU_ErrorTrap()` has 19 INFERRED edges - model-reasoned connections that need verification._
- **What connects `R_FrameChunk`, `R_Mixer`, `RingbufferView Test` to the rest of the system?**
  _111 weakly-connected nodes found - possible documentation gaps or missing edges._