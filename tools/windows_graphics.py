#!/usr/bin/env python3
"""Provide a pinned per-process Mesa driver for Windows graphics tests."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import urllib.request

VERSION = "26.2.6"
ARCHIVE = f"mesa3d-{VERSION}-release-mingw.7z"
URL = f"https://github.com/pal1000/mesa-dist-win/releases/download/{VERSION}/{ARCHIVE}"
DRIVER_FILES = ("opengl32.dll", "libgallium_wgl.dll")
SHA256 = "da0f71d9181b672c1918f3c342f11118c687f6059a46b16502845160733cc388"


def prepare(directory):
    directory.mkdir(parents=True, exist_ok=True)
    archive = directory / ARCHIVE
    if not archive.exists():
        with urllib.request.urlopen(URL, timeout=120) as source, archive.open("wb") as output:
            shutil.copyfileobj(source, output)
    with archive.open("rb") as source:
        digest = hashlib.file_digest(source, "sha256").hexdigest()
    if digest != SHA256:
        archive.unlink()
        raise RuntimeError("Mesa archive checksum mismatch")
    extracted = directory / "driver"
    if not (extracted / "x64/opengl32.dll").is_file():
        extracted.mkdir(exist_ok=True)
        subprocess.run(["cmake", "-E", "tar", "xf", str(archive.resolve())],
                       cwd=extracted, timeout=120, check=True)
    driver = extracted / "x64"
    if not (driver / "opengl32.dll").is_file() or not (driver / "libgallium_wgl.dll").is_file():
        raise RuntimeError("Mesa archive has no x64 WGL software driver")
    return driver


def deploy(directory):
    driver = Path(os.environ["MALVADO_MESA_DIR"])
    for name in DRIVER_FILES:
        if not (driver / name).is_file():
            raise RuntimeError(f"Mesa driver is incomplete: {name}")
    for name in DRIVER_FILES:
        source = driver / name
        target = directory / name
        if (target.exists() and target.stat().st_size == source.stat().st_size
                and target.stat().st_mtime_ns == source.stat().st_mtime_ns):
            continue
        shutil.copy2(source, target)


def main():
    if len(sys.argv) < 3:
        raise SystemExit("Usage: windows_graphics.py prepare DIRECTORY | run EXECUTABLE [ARGS]")
    if sys.argv[1] == "prepare":
        print(prepare(Path(sys.argv[2])))
    elif sys.argv[1] == "run":
        executable = Path(sys.argv[2]).resolve()
        deploy(executable.parent)
        raise SystemExit(subprocess.run([str(executable), *sys.argv[3:]], timeout=120).returncode)
    else:
        raise SystemExit("Unknown graphics driver operation")


if __name__ == "__main__":
    main()
