#!/usr/bin/env python3
"""Prepare checksum-pinned Linux Android emulator inputs in an owned directory."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import hashlib
import os
from pathlib import Path, PurePosixPath
import shutil
import stat
import tempfile
import urllib.request
import zipfile

ARCHIVES = (
    ("emulator-linux_x64-16740729.zip", "cbc7c3cfb83220b8a0f9c513fdab557712e74d339ec19ddffa43f23350ae9bdd"),
    ("sys-img/google_apis/x86_64-34_r14.zip", "783a40134baf4f3012d4464fbe1571b1612a0dbd2e7a44d14bd8328923443833"),
)


def extract(archive, output, expected):
    digest = hashlib.sha256()
    with archive.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    if digest.hexdigest() != expected:
        raise ValueError("Android archive SHA-256 mismatch")
    with zipfile.ZipFile(archive) as bundle:
        # Validate every entry before extraction. Symlinks cannot redirect writes.
        for entry in bundle.infolist():
            name = PurePosixPath(entry.filename)
            if name.is_absolute() or ".." in name.parts or "\\" in entry.filename:
                raise ValueError("Unsafe Android archive path")
            mode = entry.external_attr >> 16
            if stat.S_ISLNK(mode):
                raise ValueError("Android archive symlinks are not supported")
        for entry in bundle.infolist():
            target = output / entry.filename
            if entry.is_dir():
                target.mkdir(parents=True, exist_ok=True)
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                with bundle.open(entry) as source, target.open("wb") as destination:
                    shutil.copyfileobj(source, destination)
                target.chmod(0o755 if (entry.external_attr >> 16) & 0o111 else 0o644)


def prepare(output):
    if output.exists():
        raise ValueError("Android archive destination already exists")
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="android-archives-", dir=output.parent) as temporary:
        stage = Path(temporary) / "sdk"
        stage.mkdir()
        for index, (name, digest) in enumerate(ARCHIVES):
            archive = Path(temporary) / f"{index}.zip"
            with urllib.request.urlopen("https://dl.google.com/android/repository/" + name, timeout=60) as source:
                with archive.open("wb") as destination:
                    shutil.copyfileobj(source, destination)
            extract(archive, stage, digest)
            archive.unlink()
        for required in ("emulator/emulator", "x86_64/system.img", "x86_64/source.properties"):
            if not (stage / required).is_file():
                raise ValueError(f"Incomplete Android archives: {required}")
        # Keep the destination exclusive, including if another producer races us.
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
    prepare(args.output.resolve())
