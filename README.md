# `noodnik2`'s Fork of [Nuked-SC55](https://github.com/jcmoyer/Nuked-SC55)

Below are my notes related to this great and inspiring upstream project which I've become aware of in my search for
useful MIDI tooling across the Gitosphere.  

The "upstream" project's `README` can be found [here](./README-upstream.md).

## My Focus

What differentiates _my_ fork from its predecessors (i.e. [J.C. Moyer's fork](https://github.com/jcmoyer/Nuked-SC55)
of [`nukeykt`'s original Nuked-SC55](https://github.com/nukeykt/Nuked-SC55) project) is the focus on rendering
MIDI files so that they can be played later through an audio player such as the Apple Music app or iTunes.  While
J.C. Moyer's fork gets me part of the way there (e.g., to `.wav` files), I'm working on orchestrating the rendering
of a large number of MIDI files into a single album of `.m4a` files, so they can be optimized for playback by a
larger set of audio players, and in particular uploaded into the Apple Music app or iTunes so that I can enjoy my
MIDI music from my phone, computer, or Sonos speakers.

I feel much appreciation for the original works on which this personalized variant is based!

- _**NOTE**: I'm only using macOS as of now; none of what I'm developing - at least as of this writing - is
  aimed at being cross-platform, though I would be interested in helping with that if there's interest._

## Project Folder Structure

To help orient the reader, here's a quick overview of the project's folder structure: 

- `doc` - Some more documentation worth checking out.
- `src` - C++ source code.  If you modify the code, you'll be touching the files in this folder.
- `scripts` - Some utility scripts; e.g., for rendering MIDI files into audio files.
- `build` - When you "build" the Nuked-SC55 code, its executables will be written into this folder.
- `dist` - Folder into which executables and scripts are collected to make it easy to run everything
           from one place (aka "distribution" folder).
- `share` - User-managed folder containing files that are needed to run the project.
  - `midis` - Source collection of MIDI files (e.g., symbolic link).
  - `lists` - Managed list of MIDI files (e.g., favorites, etc.)
    - `favorites.list` _(example)_
  - `nuked-sc55` - SC55 ROM files (see below)
    - `rom1.bin`
    - `rom2.bin`
    - `rom_sm.bin`
    - `waverom1.bin`
    - `waverom2.bin`
  - `output` - Folder used for (intermediate) output files (e.g., rendered output)
    - `favorites.album` _(example)_
    - `test.wav` _(example)_

### NOTES

- You (the user) will need to create and manage the contents of the `share` folder, using the structure depicted
  above as a guide.
- The [make] tool can be used to advance the workflow from building to running the Nuked-SC55 application. 
  Type `make` to see the available targets and their descriptions.
- The [BUILDING] doc contains more technical information about building the Nuked-SC55 executables.

## Building The Applications

Here are my notes below about building from the source code - mostly an addendum to the (original) [BUILDING](./BUILDING.md) doc.

Note that I've only been using macOS, so my changes here will likely not work on - and are likely to have broken 
support for - other platforms.

### Requirements

The minimum set of prerequisite tools, resources, and libraries needed for building and using this project in macOS,
as described in the later sections and elsewhere across the repository, is:

- [make] – Automates building software from code; installs with `brew install make`.
- [cmake] – Cross-platform build system generator; installs with `brew install cmake`.
- [rtmidi] – A C++ library for MIDI I/O, needed when building the application; installs with `brew install rtmidi`.
- [playmidi] – Command line MIDI file player for macOS; copy or build from the source repository.
- [ffmpeg] – Audio converter toolkit; follow installation instructions at website.

### Using the Makefile

As I'm a longtime user of the [make] tool, I've once again leveraged it to help me conveniently store and later
invoke useful, frequently used sets of actions during my development workflow.  Those sets of actions are defined
in the [Makefile](./Makefile) file and are called "targets."

Assuming the prerequisites listed above are installed on your macOS system, typing `make` into a shell console at
the root of the project will list the available targets.

### Full Build

The `build` and `dist` targets of the [Makefile](./Makefile) should be enough to build the project and create the
distribution folder from where the application can be invoked using [make] on macOS; e.g.:

```shell
$ make build dist
```

If the command above completes successfully, you should be able to run the application through the commands found
in the `dist` folder.  For example, to run Nuked-SC55 in its "interactive" mode:

```shell
$ dist/nuked-sc55
```

#### Under the Hood

Also, check out the more basic build procedures described in [BUILDING].

## Running The Applications

### Installing the ROMs

Before Nuked-SC55 can be run, the ROM files it needs (which are identified in [BUILDING]) must be obtained.
Since they contain proprietary information, you'll need to provide them yourself.

By default, Nuked-SC55 looks for the ROMs in the folder `../share/nuked-sc55` relative to its location.
Therefore, assuming you run `nuked-sc55` from the `dist` folder (as in the examples below), it will look
for them in the sub-folder `share/nuked-sc55`.  However, you can specify the path to a different folder
containing the ROMs using the `-d` (or `--rom-directory`) command-line argument.

After getting the ROMs, be sure to verify them by comparing their expected hashes to those listed in
the [BUILDING] doc (e.g., using `shasum -a 256 share/nuked-sc55/*.bin`).


### Use Cases

#### MIDI Player

Because Nuked-SC55 is a MIDI synthesizer, when used "interactively" (i.e., after having started Nuked-SC55 in
"interactive mode" as described in the section below), it relies upon a separate MIDI player (such as [playmidi],
[SendMIDI] or [MidiPipe]), controller, or sequencer to generate and deliver MIDI events.

I've had luck using the [playmidi] (thank you, Nitin!) CLI to read MIDI files and reliably send their event sequences
to Nuked-SC55, but any other standard MIDI source should work, provided the routing is set up correctly.

##### Routing MIDI Signals to the SC55

I've been able to use the standard "Audio MIDI Setup" app that's native to macOS to configure the routing of MIDI
events to a running instance of Nuked-SC55.  However, I've found it typically requires a bit of head-scratching
to get it working correctly after dropping work on this project for a while.

In my most recent experience, I've found that querying to discover the available MIDI ports first, followed by
using the preferred input and output devices as listed there is generally effective.  For example:

```shell
$ dist/nuked-sc55 --help
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

From the above, we note that we can send MIDI events to the Nuked-SC55 device through MIDI input Bus `1` or `2`.
And, to route the sound through my MacBook's speakers, I'll direct the output to the corresponding output Bus `2`.

##### Starting the Nuked-SC55 in "interactive" mode

In its "interactive" mode, Nuked-SC55 acts as a MIDI synthesizer by responding to MIDI events it receives on the
input Buses it listens to.  Its standard user interface presents an LCD representing the physical device it's
emulating while listening for, and responding, to user input from the keyboard.

For a better idea of what keys are available to control the Nuked-SC55 application while it's running in "interactive"
mode, check out the source code: e.g., [lcd_sdl.cpp](src/standard/lcd_sdl.cpp).

An example command that can be used to start the Nuked-SC55 in interactive mode:

```shell
$ dist/nuked-sc55 --mk2 -p 2 -a 2
```

- `-p` - listens for MIDI events on the input Bus `2`.
- `-a` - routes the sound to the MacBook's speakers.

See the [documentation for running in this mode](./doc/standard_frontend.md) in the `doc` folder.

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

The "rendering" use case supports the transcoding of MIDI files so they can be played later through an audio player
such as the Apple Music app or iTunes, Sonos, etc.

##### Rendering to `.wav` Files

An extended functionality of Nuked-SC55 provided by [this upstream fork](https://github.com/jcmoyer/Nuked-SC55)
(of the [original Nuked-SC55](https://github.com/nukeykt/Nuked-SC55) project)
is to render MIDI files into `.wav` files.

See the [documentation for running in `renderer` mode](./doc/renderer_frontend.md) in the `doc` folder.

##### Enhanced Rendering 

Scripts are used to facilitate additional rendering of MIDI files to `.m4a` files, which can be played by Apple Music
and a large variety of other audio players.

Check out the following scripts:

- [mid2wav.sh](./scripts/mid2wav.sh) - Renders a single MIDI file into `.wav` format.
- [wav2m4a.sh](./scripts/wav2m4a.sh) - Renders a `.wav` file into `.m4a` format using [ffmpeg].
- [create-m4a-album.sh](./scripts/create-m4a-album.sh) - Creates an album of `.m4a` files from a list of MIDI files

For example, I was able to create a playable "album" of my favorite MIDI files on Apple Music after preparing a list
of their file names, then using the scripts above to transcode them into a folder of `.m4a` files which I uploaded
to Apple Music.

How I did this:

```shell
$ # create a list of all my MIDI files
$ find share/midis/ -type f -name "*.mid" > favorites.list
$ # edit the list to remove any files I don't want to include in the album
$ edit favorites.list
$ # create the "album" folder into which to write the `.m4a` files to be created
$ mkdir -p favorites.album
$ # transcode the MIDI files into `.m4a` files 
$ scripts/create-m4a-album.sh favorites.album $(cat favorites.list)
$ # open the "album" folder in Finder and click on one, or upload them all to Apple Music, etc. 
$ open favorites.album
```

Next, by using the "Add to Album" feature of Apple Music, I was able to create a new album containing the `.m4a` files
created in the step depicted above.  After watching these files get synchronized to the Cloud within my Apple Music
account, I was then able to enjoy playing them through my phone, computer, and Sonos speakers!


[BUILDING]: ./BUILDING.md
[make]: https://formulae.brew.sh/formula/make
[cmake]: https://formulae.brew.sh/formula/cmake
[playmidi]: https://github.com/nitinseshadri/playmidi
[SendMIDI]: https://github.com/gbevin/SendMIDI
[MidiPipe]: http://www.subtlesoft.square7.net/MidiPipe.html
[rtmidi]: https://github.com/thestk/rtmidi
[ffmpeg]: https://www.ffmpeg.org/
