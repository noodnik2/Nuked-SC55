# `noodnik2`'s Fork of [Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55)

Here are my notes related to this great and inspiring upstream project which I've become
aware of in my search for useful MIDI tooling across the Gitosphere.  I feel much appreciation
for the original works on which this personalized variant is based!

For more context, take a look at [this repo's copy of the upstream project's `README` file](./README-upstream.md).

## Building

Here are my notes about building from the source code - mostly an addendum to the (original) [BUILDING](./BUILDING.md) doc.

Note that I've only been using macOS, so my changes here will likely not work on - and are likely to have broken 
support for - other platforms.

### Requirements

Some prerequisite tools, resources or libraries are either required or otherwise recommended to have on-hand
for building or using this project in macOS, as described below and elsewhere across the repository:

- [rtmidi] - A C++ library for MIDI I/O, needed when building the application; installs with `brew install rtmidi`.
- [playmidi] - Command line MIDI file player for macOS; copy or build from source repository.
- [ffmpeg] - Audio converter toolkit; follow installation instructions at website. 

### Full Build

The `build` target of the [Makefile](./Makefile) should be enough to build the project. 

See the more basic build procedure described in [BUILDING]; e.g.:

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

Before Nuked-SC55 can be run, the ROM files it needs (which are identified in [BUILDING]) must be obtained.
Since they contain proprietary information, you'll need to provide them yourself.

By default, Nuked-SC55 looks for the ROMs in the `share/nuked-sc55` folder, relative to the project root.
However, you can specify the path to a different folder containing the ROMs using the `-d` (or
`--rom-directory`) command-line argument.

After obtaining the ROMs, be sure to verify them by comparing their expected hashes to those listed in
the [BUILDING] doc (e.g., using the `shasum -a 256 share/nuked-sc55/*.bin` command).


### Use Cases

#### MIDI Player

Because Nuked-SC55 is a MIDI synthesizer, when used "interactively", it relies upon a separate MIDI player
(such as [playmidi], [SendMIDI] or [MidiPipe]), controller, or sequencer to generate & send MIDI events to it.

I've had luck using the [playmidi] (thank you, Nitin!) CLI to read MIDI files and reliably send their sequence
of events to Nuked-SC55, but any other standard MIDI source should work, provided the routing is set up correctly.

##### Routing MIDI Signals to the SC55

I've been able to use the standard "Audio MIDI Setup" app that's native to macOS to configure the routing of
MIDI signals to a running instance of Nuked-SC55.  However, it typically requires a bit of head-scratching
to get it working correctly after dropping work on this project for a while.

In my most recent experience, I've found that querying to discover the available MIDI ports first, followed by
using the preferred input and output as listed there is generally effective.  For example:

```shell
$ ./build/nuked-sc55 --help
...
Known midi devices:

  0: Network Session
  1: Nuked-SC55 Bus
  2: Nuked-SC55 Bus

Known output devices:

  0: LG HDR 4K
  1: BlackHole 2ch
  2: MacBook Pro Speakers
  3: Aggregate Device
```

From the above, we note that we can send MIDI events to the Nuked-SC55 device through MIDI input Bus `2` (or `1`).
And, to route the sound through my MacBook's speakers, I'll direct the output to the corresponding output Bus `2`.

##### Starting the Nuked-SC55 in "interactive" mode

After understanding the routing configuration as described above, we're ready to start the Nuked-SC55 in its
"interactive" mode.  In this mode, it presents an LCD-style user interface representing the physical device
it emulates, listens to and responds to user input from the keyboard and also acts as a MIDI synthesizer,
responding to MIDI events it receives on the input Buses it's listening to.

An example command that can be used to start the Nuked-SC55 in interactive mode:

```shell
$ ./build/nuked-sc55 -d build --mk2 -p 2 -a 2
```

- `-d` - is used in this example because the ROMs are located in the `build` folder in my set up.
- `-p` - listens for MIDI events on the input Bus `2`.
- `-a` - routes the sound to the Macbook's speakers.

##### Playing MIDI Files

After starting the Nuked-SC55 in "interactive" mode, send the MIDI events to it using a MIDI generator, taking care
to route the events to the same MIDI Bus (port) as the Nuked-SC55 server is listening on. 

For example, to play a MIDI file using [playmidi], directing MIDI events to bus `2`, we could:

```shell
$ playmidi midis/gs/55sex.mid 2
```

##### Gotchas

When things aren't working as they should:

1. If the macOS MIDI Server crashes, try restarting it by exiting the "Audio MIDI Setup" app, then killing
  the `MIDIServer` process, e.g.:

```shell
$ sudo killall MIDIServer
```

The service should then restart automatically.  If that doesn't work, I suggest using your favorite AI to
help you diagnose the problem - at least, that's what's helped me in the past!

2. If you don't see any activity within the Nuked-SC55 simulated LCD screen in response to MIDI events,
  check the routing configuration through the available ports in the "Audio MIDI Setup" app.

#### Rendering

An interesting use case for me is to _render_ MIDI files into audio files so that I can enjoy them on my phone,
through Sonos, or in my car.

Using a pair of scripts, I was able to produce several "albums" comprising sets of `.m4a` (Apple AAC) files for easy
distribution to Apple Music (e.g., for playback via my personal account), or via SSD or cloud storage.

For example, after preparing a list of (the names of) my favorite MIDI files, I rendered them first into `.wav` files,
then compressed those into `.m4a` format using [ffmpeg] before opening them in Apple Music.

From the `build` folder:

```shell
$ ../scripts/create-m4a-album.sh favorites.album ~/midis/favorites/*.mid
$ open favorites.album
```

Next, by using the "Add to Album" feature of Apple Music, I was able to create a new album containing the `.m4a` files
created in the step depicted above.  After watching these files get synchronized to the Cloud within my Apple Music
account, I was then able to find and play from my phone and in my Sonos' Apple Music (synchronization) folder!


[BUILDING]: ./BUILDING.md
[playmidi]: https://github.com/nitinseshadri/playmidi
[SendMIDI]: https://github.com/gbevin/SendMIDI
[MidiPipe]: http://www.subtlesoft.square7.net/MidiPipe.html
[rtmidi]: https://github.com/thestk/rtmidi
[ffmpeg]: https://www.ffmpeg.org/
