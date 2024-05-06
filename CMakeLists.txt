cmake_minimum_required (VERSION 3.15)
project (NukedSC55 VERSION 0.2.0 LANGUAGES CXX)

#==============================================================================
# Dependencies
#==============================================================================
find_package(SDL2 REQUIRED)

if(NOT WIN32)
    set(USE_RTMIDI TRUE)
endif()

if(USE_RTMIDI)
    find_package(rtmidi REQUIRED)
endif()

#==============================================================================
# Support functions
#==============================================================================
function(target_enable_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W4)
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

#==============================================================================
# Backend
#==============================================================================
add_library(nuked-sc55-backend ${SC55_BACKEND_SRC})
target_sources(nuked-sc55-backend
    PRIVATE
    src/mcu.cpp src/mcu.h
    src/lcd.cpp src/lcd.h src/lcd_font.h
    src/mcu_interrupt.cpp src/mcu_interrupt.h
    src/mcu_opcodes.cpp src/mcu_opcodes.h
    src/mcu_timer.cpp src/mcu_timer.h
    src/midi.h
    src/pcm.cpp src/pcm.h
    src/submcu.cpp src/submcu.h
    src/emu.cpp src/emu.h
    src/ringbuffer.h
    src/math_util.h
)
target_link_libraries(nuked-sc55-backend PRIVATE SDL2::SDL2)
target_compile_features(nuked-sc55-backend PRIVATE cxx_std_23)
target_enable_warnings(nuked-sc55-backend)

#==============================================================================
# Frontend
#==============================================================================
add_executable(nuked-sc55 ${SC55_FRONTEND_SRC} ${UTF8MAIN_SRCS})
target_sources(nuked-sc55
    PRIVATE
    src/main_frontend.cpp # main() is here!
)

if(USE_RTMIDI)
    target_sources(nuked-sc55 PRIVATE src/midi_rtmidi.cpp)
elseif(WIN32)
    target_sources(nuked-sc55 PRIVATE src/midi_win32.cpp)
endif()

target_link_libraries(nuked-sc55 PRIVATE SDL2::SDL2 SDL2::SDL2main nuked-sc55-backend)
target_compile_features(nuked-sc55 PRIVATE cxx_std_23)
target_enable_warnings(nuked-sc55)

if(USE_RTMIDI)
    target_link_libraries(nuked-sc55 PRIVATE RtMidi::rtmidi)
endif()

if(WIN32)
    target_link_libraries(nuked-sc55 PRIVATE shlwapi winmm)
endif()

if(APPLE)
    find_library(LIBCoreAudio CoreAudio)
    target_link_libraries(nuked-sc55 PRIVATE ${LIBCoreAudio})
endif()

