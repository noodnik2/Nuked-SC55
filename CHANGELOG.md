# Version 0.5.0 (TBD)

## Enhancements

- The standard frontend will now always respond to user input even if the
  emulator starts falling behind.

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
