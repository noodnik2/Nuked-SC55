#!/usr/bin/env bash

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
RENDER_TOOL=nuked-sc55-render
RENDER_CMD=$(command -v "$RENDER_TOOL")
if [[ -z "$RENDER_CMD" || ! -x "$RENDER_CMD" ]]; then
  SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
  RENDER_CMD="${SCRIPT_DIR}/${RENDER_TOOL}"
fi

[[ -x "$RENDER_CMD" ]] || fatal "${RENDER_TOOL} not found"

# Confirm the output dir either exists and is empty, or is created
[[ ! -d "$output_dir" || -z "$(ls -A "$output_dir")" ]] && mkdir -p "$output_dir" || fatal "error: '$output_dir' exists and is not empty"

# Set up fast fail
set -euo pipefail

# Render the .mid(s) to .wav(s)
for midFilePath in "${mid_files[@]}"; do
  midFileName="${midFilePath##*/}"
  wavFileName="${midFileName%.*}.wav"
  wavFilePath="$output_dir/$wavFileName"
  echo "rendering '$wavFilePath' from '$midFilePath'"
  $RENDER_CMD -r gs "$midFilePath" -o "$wavFilePath" || fatal "rendering error"
done

