#!/usr/bin/env python3
"""Install and relocate the graphical SDK, then verify the Rust author workflow."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--visual", action="store_true")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="malvado-rust-cli-") as directory:
        work = Path(directory)
        staged = work / "staged"
        prefix = work / "relocated sdk"
        subprocess.run(["cmake", "--install", str(ROOT / "build"), "--prefix", str(staged)],
                       check=True, stdout=subprocess.DEVNULL)
        staged.rename(prefix)
        command = ["python3", str(ROOT / "tests/rust_cli.py"),
                   "--cli", str(prefix / "bin/malvado"), "--work", str(work / "projects")]
        if args.visual:
            command.append("--visual")
        subprocess.run(command, cwd=work, check=True)


if __name__ == "__main__":
    main()
