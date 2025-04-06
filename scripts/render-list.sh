#! /bin/bash

# Render the `.mid` files listed in `$FAVORITES_LIST` found within `$MIDI_COLLECTION_DIR` into `.wav` format

stderr() {
  echo "$@" 1>&2
}

fatal() {
  stderr "$@"
  exit 1
}

# the following values can be overridden from exported value; or, will use default
FAVORITES_LIST=${FAVORITES_LIST:-../share/lists/favorites.list}
MIDI_COLLECTION_DIR=${MIDI_COLLECTION_DIR:-.}

[ -f $FAVORITES_LIST ] || fatal "not found: $FAVORITES_LIST"

for cmd in ./nuked-sc55-render basename; do
  which $cmd > /dev/null || fatal "$cmd not found"
done

for f in $(cat $FAVORITES_LIST); do
  midiFilePath="$MIDI_COLLECTION_DIR/$f"
  if [ ! -f "$midiFilePath" ]; then
    stderr "skipping: '$midiFilePath'; not found"
    continue
  fi

  waveFileName="$(basename "$midiFilePath" .mid).wav"
  echo "rendering '$waveFileName' from '$midiFilePath'"
  ./nuked-sc55-render -r gs "$midiFilePath" -o "$waveFileName" || fatal "rendering error"
done