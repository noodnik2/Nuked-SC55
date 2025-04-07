#!/usr/bin/env bash

set -euo pipefail

stderr() {
  echo "$@" 1>&2
}

fatal() {
  stderr "$@"
  exit 1
}

# Parse & validate argument(s)

[ $# -lt 2 ] && fatal "Usage:
  $0 <output-folder-name> ifn [ifn [...]]
  (Where the input file name(s) must end either in .mid or .wav)"

output_dir="$1"
shift; inputs=("$@")
wav_files=()

for file in "${inputs[@]}"; do
  [ -f $file ] || fatal "file '$file' not found"
  case "$file" in
    *.wav|*.WAV) wav_files+=("$file") ;;
    *) fatal  "Unsupported file extension: $file" ;;
  esac
done

# Check dependencies
FFMPEG_CMD=ffmpeg
for cmd in date $FFMPEG_CMD; do
  which $cmd > /dev/null || fatal "$cmd not found"
done

# Confirm the output dir either exists and is empty, or is created
[[ ! -d "$output_dir" || -z "$(ls -A "$output_dir")" ]] && mkdir -p "$output_dir" || fatal "error: '$output_dir' exists and is not empty"

# Render the .wav(s) to .m4a(s)

track_no=1
full_batch_name="${output_dir} ($(date +%y%m%d%H%M))"
echo "started '${full_batch_name}'"

for wavFilePath in "${wav_files[@]}"; do
  wavFileName="${wavFilePath##*/}"
  m4aFileName="${wavFileName%.*}.m4a"
  m4aFilePath="$output_dir/$m4aFileName"
  echo "rendering '$m4aFilePath' from '$wavFilePath'"
  $FFMPEG_CMD -i "$wavFilePath" -c:a aac -b:a 256k -y \
    -metadata title="$wavFileName" \
    -metadata album="$full_batch_name" \
    -metadata track="$track_no" \
    "$m4aFilePath" || fatal "rendering error"
  track_no=$((track_no + 1))
done

echo "finished '${full_batch_name}'"
