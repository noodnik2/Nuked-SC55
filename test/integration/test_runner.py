import subprocess
import argparse
import hashlib
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--render-exe", type=str)
parser.add_argument("--input", type=str)
parser.add_argument("--sha256", type=str)
parser.add_argument("--rom-directory", type=str)


def main():
    args = parser.parse_args()

    cmd = [args.render_exe, "--stdout", args.input, "-d", args.rom_directory]

    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as proc:
        digest = hashlib.file_digest(proc.stdout, "sha256")

    if digest.hexdigest().casefold() != args.sha256.casefold():
        print(f"hash mismatch for {args.input}")
        sys.exit(1)

    sys.exit(proc.wait())


if __name__ == "__main__":
    main()
