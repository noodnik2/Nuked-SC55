# `noodnik2`'s Fork of [Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55)

Here are my notes related to this great and inspirational upstream project which I've become
aware of in my search for useful MIDI tooling across the Gitosphere.  I feel much appreciation
for the original works on which this personalized variant is based! 

## Building

Here are my notes about building from the source code - mostly an addendum to the (original)
[BUILDING](./BUILDING.md) doc.

### Requirements

- `rtmidi`

#### MacOS

The `rtmidi` library was missing when I first attempted to build.  I was able to fix that
using HomeBrew, e.g.:

```shell
$ brew install rdmidi
```

### Full Build

Same thing as described in [BUILDING], just recording (possibly tweaked preferences & notes):

```shell
$ mkdir -p build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-march=native -mtune=native" \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  ..
$ cmake --build .  
```

## Running

### Installing the ROMs

I see that the tool supports reading the ROMs it needs from the `./share/nuked-sc55` folder,
so I added that to the `.gitignore` and put my files there.

### Use Cases

#### Rendering

My first foray into using the tools available in this repo was to render some of my favorite
MIDIs so that I can add them into albums with my Apple Music account.  Using a pair of scripts,
I was able to produce several albums and simple `open` the "album" folders I copied the resulting
`.m4a` (Apple AAC) files into.

For example, after preparing a list of (the names of) my favorite MIDI files, I rendered them first
into `.wav` files, then compressed those (using `ffmpeg`) before opening them in Apple Music.  From
the `build` folder:

```shell
$ ../scripts/render-list.sh
$ ../scripts/compress.sh 'Favorite MIDI Files'
$ mkdir favorites.album
$ mv *.m4a favorites.album
$ open favorites.album
```

After selecting and opening all the files with "Apple Music", I had a new album!

It's pretty cool to be able to play these now anywhere I go - on my iPhone, in my car, etc!


[BUILDING]: ./BUILDING.md