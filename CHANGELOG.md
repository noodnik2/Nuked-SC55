# Version 0.5.0 (TBD)

## Enhancements

- The standard frontend will now always respond to user input even if the
  emulator starts falling behind.
- The renderer frontend will now run the emulator for a bit before sending any
  MIDI events even if you don't specify a reset with `-r` or `--reset`. This
  should ensure it's ready to accept program changes. (#20)
- The renderer will no longer ask the user to submit a bug report when the
  instance count is set to a number that results in only some instances having
  a midi track to render. (#19, #24)
- Added [standard frontend documentation](doc/standard_frontend.md) and
  [renderer frontend documentation](doc/renderer_frontend.md) and 
- Added ASIO support for Windows users who compile from source. See
  [BUILDING.md](BUILDING.md) for instructions. Precompiled ASIO builds cannot
  be provided because of the ASIO SDK license.
- The `-b, --buffer-size <page_size>[:page_count]` flag has been **changed** to
  actually mean buffer size. This is a divergence from upstream. The
  differences are documented
  [here](doc/standard_frontend.md#-b---buffer-size-sizecount).
- Fixed some threading soundness issues inherited from upstream that could
  theoretically cause playback issues. This is more likely to affect platforms
  with weaker memory models than x86. See [upstream
  #72](https://github.com/nukeykt/Nuked-SC55/pull/72) for some discussion about
  this.

# Version 0.4.2 (2025-02-23)

## Enhancements

- The WAV renderer now accepts MIDI files with tracks that contain extra data
  past their end-of-track event. Extra data is ignored. (#18)

# Version 0.4.1 (2025-02-17)

## Enhancements

- The WAV renderer now prints the list of accepted `--romset` names if you pass
  `--help` or provide an invalid one.

## Bugfixes

- The WAV renderer now writes correct headers when choosing the `f32` format.
  Previously, the WAV files would appear to have a long period of silence at
  the end in some audio players. (#17)

# Version 0.4.0 (2025-02-11)

Initial release
