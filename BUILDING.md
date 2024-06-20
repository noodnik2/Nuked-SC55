# Building

Requirements:

- cmake
- SDL2
- rtmidi (Linux, Mac only)

Tested compilers:

- msvc 19.39.33523
- clang 18.1.4
- gcc 13.2.0

Full build

```
git clone git@github.com:jcmoyer/Nuked-SC55.git
cd Nuked-SC55
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Windows

For builds using msvc you will most likely need to pass
`-DCMAKE_PREFIX_PATH=<path>` where `<path>` points to a directory containing
SDL2, and optionally rtmidi (only when `-DUSE_RTMIDI=ON`).

cmake is expecting to find `<path>/SDL2-X.YY.Z/cmake/sdl2-config.cmake`.

For builds in an msys2 environment, installing SDL2 via pacman should be
enough.

# Development

Requirements:

- Python 3

There is a test suite that makes sure new commits don't change existing
behavior. It is expected that all tests pass for every commit unless either:

- Upstream modified backend behavior in a way that affects sample output, or
- We modified the renderer frontend in a way that causes different output

You can run the test suite by configuring with `-DNUKED_ENABLE_TESTS=ON` and
`-DNUKED_TEST_ROMDIR=<path>` and running:

```
ctest . -C Release
```

Note that these tests take a long time to finish individually, so you may want
to pass `-j` to run them in parallel. Currently these tests require a SC-55mk2
romset with these hashes:

```
$ sha256sum rom1.bin rom2.bin rom_sm.bin waverom1.bin waverom2.bin
8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042 *rom1.bin
a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0 *rom2.bin
b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8 *rom_sm.bin
c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b *waverom1.bin
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491 *waverom2.bin
```

`NUKED_TEST_ROMDIR` should point to a directory containing these files.
