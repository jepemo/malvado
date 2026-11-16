#!/usr/bin/env python3
"""Run one native test through adb for the existing host-side CMake contracts."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
from pathlib import Path
import shlex
import subprocess
import sys

sys.dont_write_bytecode = True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--state", type=Path, required=True)
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    state = json.loads(args.state.read_text())
    prefix = state["adb"]
    arguments = args.arguments
    if arguments[:1] == ["--"]:
        arguments = arguments[1:]
    mapped = []
    for value in arguments:
        destination = None
        for source, target in state["paths"]:
            path = Path(value)
            if path.is_absolute() and path.is_relative_to(source):
                destination = target + "/" + path.relative_to(source).as_posix()
                if path.is_file():
                    subprocess.run(prefix + ["shell", shlex.join(["mkdir", "-p", str(Path(destination).parent)])],
                                   capture_output=True, check=True, timeout=15)
                    subprocess.run(prefix + ["push", str(path), destination],
                                   capture_output=True, check=True, timeout=30)
                break
        mapped.append(destination or value)
    command = "cd " + shlex.quote(state["cwd"]) + " && " + shlex.join([state["program"], *mapped])
    result = subprocess.run(prefix + ["shell", command], capture_output=True, timeout=45)
    sys.stdout.buffer.write(result.stdout)
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.SubprocessError) as error:
        print(f"Android test transport failed: {error}", file=sys.stderr)
        sys.exit(125)
