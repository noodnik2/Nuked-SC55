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
wav_files=()

for file in "${inputs[@]}"; do
  [ -f $file ] || fatal "file '$file' not found"
  case "$file" in
    *.mid|*.MID) mid_files+=("$file") ;;
    *.wav|*.WAV) wav_files+=("$file") ;;
    *) fatal "Unsupported file extension: $file" ;;
  esac
done

# Check dependencies
script_dir="${BASH_SOURCE[0]%/*}"
mid2wav_script="$script_dir/mid2wav.sh"
wav2m4a_script="$script_dir/wav2m4a.sh"
for cmd in mktemp $mid2wav_script $wav2m4a_script; do
  which $cmd > /dev/null || fatal "$cmd not found"
done

# Confirm the output dir either exists and is empty, or is created
[[ ! -d "$output_dir" || -z "$(ls -A "$output_dir")" ]] && mkdir -p "$output_dir" || fatal "error: '$output_dir' exists and is not empty"

# Convert .mid(s) to .wav(s) if needed
if [[ ${#mid_files[@]} -gt 0 ]]; then
  echo "converting .mid files to .wav..."
  tmp_wav_dir="$(mktemp -d)"
  "$mid2wav_script" "$tmp_wav_dir" "${mid_files[@]}"
  trap "rm -rf \"$tmp_wav_dir\"" EXIT
  # Add the .wav(s) created from .mid(s)
  while IFS= read -r -d '' f; do
    wav_files+=("$f")
  done < <(find "$tmp_wav_dir" -type f -name '*.wav' -print0)
fi

# Sanity check
[[ ${#wav_files[@]} -eq 0 ]] && fatal "no wav files to convert"

# Convert all .wav to .m4a
"$wav2m4a_script" "$output_dir" "${wav_files[@]}"
