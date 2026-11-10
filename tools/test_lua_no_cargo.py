#!/usr/bin/env python3
"""Configure, build, and test Lua with Cargo absent from the build PATH."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    env = dict(os.environ)
    env["PATH"] = os.pathsep.join(
        entry for entry in env["PATH"].split(os.pathsep)
        if not (Path(entry) / "cargo").is_file())
    assert shutil.which("cargo", path=env["PATH"]) is None
    for command in (
        ["cmake", "-S", ".", "-B", "build-lua-headless", "-DCMAKE_BUILD_TYPE=Debug",
         "-DMALVADO_BUILD_LUA=ON", "-DMALVADO_BUILD_RAYLIB=OFF"],
        ["cmake", "--build", "build-lua-headless", "--parallel"],
        ["ctest", "--test-dir", "build-lua-headless", "--output-on-failure"],
    ):
        subprocess.run(command, cwd=ROOT, env=env, check=True)
    print("PASS: Lua configuration, build and tests with no Cargo on PATH", flush=True)


if __name__ == "__main__":
    main()
