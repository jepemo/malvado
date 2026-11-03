#!/usr/bin/env python3
# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

"""Install or remove a locally built development SDK."""

import argparse
import json
import os
from pathlib import Path, PurePosixPath
import shlex
import shutil
import subprocess
import sys
import tempfile


MARKER = ".malvado-dev-sdk.json"


def inventory(root):
    entries = []
    def fail(error):
        raise error

    for directory, subdirectories, files in os.walk(root, followlinks=False, onerror=fail):
        for name in subdirectories + files:
            path = Path(directory) / name
            relative = path.relative_to(root).as_posix()
            if relative != MARKER:
                entries.append(relative)
    return sorted(entries)


def check_prefix(prefix):
    if prefix.is_symlink():
        raise ValueError(f"Installation path is a symbolic link: {prefix}")
    protected = {Path(prefix.anchor), Path.home().resolve(), Path(__file__).resolve().parent.parent}
    if prefix.resolve() in protected:
        raise ValueError(f"Cannot use this installation path: {prefix}")


def check_managed(prefix):
    check_prefix(prefix)
    if not os.path.lexists(prefix):
        return False
    marker = prefix / MARKER
    if not prefix.is_dir() or marker.is_symlink() or not marker.is_file():
        raise ValueError(f"Not a managed development SDK: {prefix}")
    try:
        data = json.loads(marker.read_text(encoding="utf-8"))
        if not isinstance(data, dict):
            raise ValueError
        entries = data["entries"]
        if data.get("format") != 1 or not isinstance(entries, list):
            raise ValueError
        for entry in entries:
            if not isinstance(entry, str):
                raise ValueError
            path = PurePosixPath(entry)
            if path.is_absolute() or not path.parts or any(part in (".", "..") for part in path.parts):
                raise ValueError
    except (OSError, ValueError, KeyError, TypeError) as error:
        raise ValueError(f"Invalid development SDK marker: {marker}") from error
    extra = sorted(set(inventory(prefix)) - set(entries))
    if extra:
        raise ValueError(f"Unmanaged files in {prefix}; remove or move them first: {', '.join(extra[:3])}")
    return True


def install(prefix, build):
    managed = check_managed(prefix)
    prefix.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".malvado-dev-", dir=prefix.parent) as temporary:
        temp = Path(temporary)
        stage = temp / "sdk"
        subprocess.run(["cmake", "--install", str(build), "--prefix", str(stage), "--config", "Debug"], check=True)
        executable = stage / "bin" / "malvado"
        if not executable.is_file():
            raise ValueError("Build does not contain the malvado client. Enable Lua and build again.")
        report = subprocess.run([str(executable), "doctor"], check=True, capture_output=True, text=True).stdout
        if "Graphics: enabled" not in report or "Export: launchers and required licenses found" not in report:
            raise ValueError("Build is not a complete graphical SDK. Enable Lua and raylib and build again.")
        (stage / MARKER).write_text(json.dumps({"format": 1, "entries": inventory(stage)}, indent=2) + "\n", encoding="utf-8")
        if check_managed(prefix) != managed:
            raise ValueError("Installation changed during the build; try again.")
        backup = temp / "previous"
        if managed:
            prefix.rename(backup)
        try:
            stage.rename(prefix)
        except OSError:
            if managed:
                backup.rename(prefix)
            raise
    print(f"Installed development SDK: {prefix}")
    print(f'Add it to this terminal: export PATH={shlex.quote(str(prefix / "bin"))}:"$PATH"')


def uninstall(prefix):
    if not check_managed(prefix):
        print(f"Development SDK is not installed: {prefix}")
        return
    shutil.rmtree(prefix)
    print(f"Removed development SDK: {prefix}")


def main():
    parser = argparse.ArgumentParser(description="Manage the local Malvado development SDK.")
    parser.add_argument("action", choices=("install", "uninstall"))
    parser.add_argument("--build", default="build")
    parser.add_argument("--prefix", default=os.environ.get("MALVADO_DEV_PREFIX", "~/.local/share/malvado-dev"))
    args = parser.parse_args()
    if sys.platform != "darwin" and not sys.platform.startswith("linux"):
        parser.error("Development SDK installation supports macOS and Linux.")
    prefix = Path(os.path.abspath(os.path.expanduser(args.prefix)))
    try:
        if args.action == "install":
            install(prefix, Path(args.build).resolve())
        else:
            uninstall(prefix)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Development SDK {args.action} failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
