#!/usr/bin/env python3
"""Prepare checksum-pinned Linux/macOS Android emulator inputs in an owned directory."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import hashlib
import json
import os
import posixpath
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


MACOS_ARCHIVES = (
    ("emulator-darwin_aarch64-16740729.zip", "9e50ac521c38949d412f69250c0df9a4afa684d5be54ab4b80fc0205a73f5054"),
    ("sys-img/google_apis/arm64-v8a-34_r14.zip", "bd1c323cbdd3c9dd55dde54588b298d172fd126fe16e127a7bd7aeab6897890a"),
)


def extract(archive, output, expected, *, file_symlinks=False):
    digest = hashlib.sha256()
    with archive.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    if digest.hexdigest() != expected:
        raise ValueError("Android archive SHA-256 mismatch")
    with zipfile.ZipFile(archive) as bundle:
        entries = {entry.filename.rstrip('/'): entry for entry in bundle.infolist()}
        links = {}
        if file_symlinks and output.exists() and any(output.iterdir()):
            raise ValueError("NDK extraction requires an empty destination")
        # Validate every entry before extraction. Symlinks cannot redirect writes.
        for entry in bundle.infolist():
            name = PurePosixPath(entry.filename)
            if name.is_absolute() or ".." in name.parts or "\\" in entry.filename:
                raise ValueError("Unsafe Android archive path")
            mode = entry.external_attr >> 16
            if stat.S_ISLNK(mode):
                if not file_symlinks:
                    raise ValueError("Android archive symlinks are not supported")
                target = bundle.read(entry).decode('utf-8')
                if not target or target.startswith('/') or '\\' in target:
                    raise ValueError("Unsafe Android archive symlink")
                links[entry.filename] = posixpath.normpath(posixpath.join(str(name.parent), target))
            elif stat.S_IFMT(mode) not in (0, stat.S_IFREG, stat.S_IFDIR):
                raise ValueError("Unsupported Android archive entry")
        # NDK links may refer only to files in this archive. No entry may have
        # a symlink parent, so extracting regular files never traverses a link.
        for name in entries:
            if any(str(parent) in links for parent in PurePosixPath(name).parents):
                raise ValueError("Android archive entry has a symlink parent")
        for name, target in links.items():
            visited = {name}
            while target in links:
                if target in visited:
                    raise ValueError("Cyclic Android archive symlink")
                visited.add(target)
                target = links[target]
            if target not in entries or entries[target].is_dir():
                raise ValueError("Android archive symlink must resolve to an archived file")
        for entry in bundle.infolist():
            target = output / entry.filename
            if entry.filename in links:
                continue
            elif entry.is_dir():
                target.mkdir(parents=True, exist_ok=True)
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                with bundle.open(entry) as source, target.open("wb") as destination:
                    shutil.copyfileobj(source, destination)
                target.chmod(0o755 if (entry.external_attr >> 16) & 0o111 else 0o644)
        for name, target in links.items():
            destination = output / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.symlink_to(posixpath.relpath(target, str(PurePosixPath(name).parent)))


def prepare(output, host="linux"):
    if host not in ("linux", "macos"):
        raise ValueError("Android emulator archive host must be linux or macos")
    archives = ARCHIVES if host == "linux" else MACOS_ARCHIVES
    abi = "x86_64" if host == "linux" else "arm64-v8a"
    if output.exists() or output.is_symlink():
        raise ValueError("Android archive destination already exists")
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="android-archives-", dir=output.parent) as temporary:
        stage = Path(temporary) / "sdk"
        stage.mkdir()
        for index, (name, digest) in enumerate(archives):
            archive = Path(temporary) / f"{index}.zip"
            with urllib.request.urlopen("https://dl.google.com/android/repository/" + name, timeout=60) as source:
                with archive.open("wb") as destination:
                    shutil.copyfileobj(source, destination)
            extract(archive, stage, digest)
            archive.unlink()
        for required in ("emulator/emulator", f"{abi}/system.img", f"{abi}/source.properties"):
            if not (stage / required).is_file():
                raise ValueError(f"Incomplete Android archives: {required}")
        provenance = dict(host=host, abi=abi,
                          archives=[dict(archive=name, sha256=digest) for name, digest in archives])
        (stage / "malvado-emulator-archives.json").write_text(json.dumps(provenance, indent=2) + "\n")
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
    parser.add_argument("--host", choices=("linux", "macos"), default="linux")
    args = parser.parse_args()
    prepare(args.output.absolute(), args.host)
