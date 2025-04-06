#! /bin/bash

# Compress the `.wav` files in the current directory

stderr() {
  echo "$@" 1>&2
}

fatal() {
  stderr "$@"
  exit 1
}

[ $# -eq 1 ] || fatal "must provide batch id"
batch_id="$1"

[ -z "$WAV_COLLECTION_DIR" ] && fatal "WAV_COLLECTION_DIR not set"
for cmd in ffmpeg basename date; do
  which $cmd > /dev/null || fatal "$cmd not found"
done

track_no=1
full_batch_name="MIDI '${batch_id}' ($(date +%y%m%d%H%M))"
echo "started '${full_batch_name}'"

for waveFilePath in *.wav; do
  originalName="$(basename "$waveFilePath" .wav)"
  compressedFileName="${originalName}.m4a"
  echo "track '${track_no}' is '$compressedFileName' from '$waveFilePath'"
  ffmpeg -i "$waveFilePath" -c:a aac -b:a 256k -y \
    -metadata title="$originalName" \
    -metadata album="$full_batch_name" \
    -metadata track="$track_no" \
    "$compressedFileName" || fatal "rendering error"
  track_no=$((track_no + 1))
done

echo "finished '${full_batch_name}'"

