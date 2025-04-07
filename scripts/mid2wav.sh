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
mid_files=()

for file in "${inputs[@]}"; do
  [ -f $file ] || fatal "file '$file' not found"
  case "$file" in
    *.mid|*.MID) mid_files+=("$file") ;;
    *) fatal "Unsupported file extension: $file" ;;
  esac
done

# Check dependencies
RENDER_CMD=./nuked-sc55-render
for cmd in $RENDER_CMD; do
  which $cmd > /dev/null || fatal "$cmd not found"
done

# Confirm the output dir either exists and is empty, or is created
[[ ! -d "$output_dir" || -z "$(ls -A "$output_dir")" ]] && mkdir -p "$output_dir" || fatal "error: '$output_dir' exists and is not empty"

# Render the .mid(s) to .wav(s)
for midFilePath in "${mid_files[@]}"; do
  midFileName="${midFilePath##*/}"
  wavFileName="${midFileName%.*}.wav"
  wavFilePath="$output_dir/$wavFileName"
  echo "rendering '$wavFilePath' from '$midFilePath'"
  $RENDER_CMD -r gs "$midFilePath" -o "$wavFilePath" || fatal "rendering error"
done

