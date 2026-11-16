#!/usr/bin/env python3
"""Install checksum-pinned Linux Android SDK inputs in a new owned directory."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
from pathlib import Path
import shutil
import sys
import tempfile
import urllib.request

sys.dont_write_bytecode = True
from android_archives import extract

PACKAGES = (
    ("android-ndk-r27c-linux.zip", "59c2f6dc96743b5daf5d1626684640b20a6bd2b1d85b13156b90333741bad5cc",
     "android-ndk-r27c", "ndk/27.2.12479018", "build/cmake/android.toolchain.cmake", True),
    ("platform-tools_r37.0.1-linux.zip", "d230f13842f60f782a8645f9c813f8f845bf36089ea7289f28c48f17979313f1",
     "platform-tools", "platform-tools", "adb", False),
    ("build-tools_r34-linux.zip", "e858c4b60069d0431051b225d384413b1643e1289b00a4825aed347f25bd510f",
     "android-14", "build-tools/34.0.0", "aapt", False),
    ("platform-34-ext7_r03.zip", "16fdb74c55e59ae3ef52def135aec713508467bd56d7dabcd8c9be31fa8b20f3",
     "android-34", "platforms/android-34", "android.jar", False),
)


def prepare(output):
    if output.exists() or output.is_symlink():
        raise ValueError("Android SDK destination already exists")
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="android-sdk-", dir=output.parent) as temporary:
        temporary = Path(temporary)
        stage = temporary / "sdk"
        stage.mkdir()
        provenance = []
        for index, (name, digest, source, destination, required, links) in enumerate(PACKAGES):
            archive = temporary / f"{index}.zip"
            with urllib.request.urlopen("https://dl.google.com/android/repository/" + name, timeout=60) as response:
                with archive.open("wb") as stream:
                    shutil.copyfileobj(response, stream)
            unpacked = temporary / f"unpacked-{index}"
            extract(archive, unpacked, digest, file_symlinks=links)
            package = unpacked / source
            if not (package / required).is_file() or not (package / "source.properties").is_file():
                raise ValueError(f"Incomplete Android SDK archive: {name}")
            target = stage / destination
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(str(package), target)
            archive.unlink()
            shutil.rmtree(unpacked)
            provenance.append(dict(archive=name, sha256=digest, destination=destination))
        (stage / "malvado-archives.json").write_text(json.dumps(provenance, indent=2) + "\n")
        # Claim the final directory exclusively. Never replace another SDK.
        output.mkdir()
        try:
            for child in stage.iterdir():
                shutil.move(str(child), output / child.name)
        except BaseException:
            shutil.rmtree(output)
            raise


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    prepare(args.output.absolute())
