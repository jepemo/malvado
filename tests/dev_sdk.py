#!/usr/bin/env python3
# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

"""Check installation, replacement, and safe removal of the development SDK."""

import argparse
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True)
    parser.add_argument("--build", required=True)
    args = parser.parse_args()
    script = Path(args.source) / "tools" / "dev_sdk.py"

    with tempfile.TemporaryDirectory(prefix="malvado-dev-test-") as temporary:
        root = Path(temporary)
        prefix = root / "SDK with spaces"

        def call(action, target=prefix, build=args.build, success=True):
            result = subprocess.run(
                [sys.executable, str(script), action, "--prefix", str(target), "--build", str(build)],
                capture_output=True, text=True,
            )
            if (result.returncode == 0) != success:
                raise AssertionError(f"{action}: {result.stdout}\n{result.stderr}")
            return result

        call("install")
        client = prefix / "bin" / "malvado"
        version = subprocess.run([str(client), "--version"], check=True, capture_output=True, text=True).stdout
        assert version.startswith("malvado ")
        project = root / "project"
        project.mkdir()
        (project / "main.lua").write_text("print('installed client works')\n", encoding="utf-8")
        run = subprocess.run([str(client), "run", "--headless", str(project)], check=True, capture_output=True, text=True)
        assert "installed client works" in run.stdout

        call("install")
        assert client.is_file()
        extra = prefix / "my-project" / "keep.txt"
        extra.parent.mkdir()
        extra.write_text("keep", encoding="utf-8")
        call("install", success=False)
        call("uninstall", success=False)
        assert extra.read_text(encoding="utf-8") == "keep" and client.is_file()
        extra.unlink()
        extra.parent.rmdir()
        call("install", build=root / "missing-build", success=False)
        assert client.is_file()
        call("uninstall")
        call("uninstall")
        assert not prefix.exists()

        unmanaged = root / "unmanaged"
        unmanaged.mkdir()
        (unmanaged / "keep.txt").write_text("keep", encoding="utf-8")
        call("install", target=unmanaged, success=False)
        call("uninstall", target=unmanaged, success=False)
        assert (unmanaged / "keep.txt").read_text(encoding="utf-8") == "keep"
        link = root / "link"
        link.symlink_to(unmanaged, target_is_directory=True)
        call("install", target=link, success=False)
        call("uninstall", target=link, success=False)
        assert (unmanaged / "keep.txt").is_file()
    print("Development SDK installation checks passed")


if __name__ == "__main__":
    main()
