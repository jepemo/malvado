#!/usr/bin/env python3
"""Build and test a native headless SDK in Debug and Release."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(log, *args, env=None, timeout=600):
    log.write(f"\n>>> {args!r}\n")
    log.flush()
    subprocess.run(args, cwd=ROOT, env=env, stdout=log,
                   stderr=subprocess.STDOUT, check=True, timeout=timeout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work", type=Path, default=ROOT / "build-platform")
    parser.add_argument("--rust", action="store_true", help="Test Rust core and assets against relocated SDKs")
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    for config in ("Debug", "Release"):
        build = work / config
        with (work / f"{config}.log").open("w", encoding="utf-8") as log:
            run(log, "cmake", "-S", str(ROOT), "-B", str(build),
                f"-DCMAKE_BUILD_TYPE={config}", "-DMALVADO_BUILD_RAYLIB=OFF",
                "-DMALVADO_BUILD_LUA=ON", "-DMALVADO_BUILD_ASSETS=ON",
                "-DCMAKE_INSTALL_LIBDIR=lib",
                # Existing C fixtures use assert() to execute operations.
                "-DCMAKE_C_FLAGS_RELEASE=" + ("/O2 /UNDEBUG" if os.name == "nt" else "-O3 -UNDEBUG"))
            run(log, "cmake", "--build", str(build), "--config", config, "--parallel")
            run(log, "ctest", "--test-dir", str(build), "-C", config,
                "--output-on-failure", "--timeout", "60")
            staged = build / "staged sdk"
            prefix = build / "relocated sdk"
            # Remove only installations owned by this verifier.
            for path in (staged, prefix):
                if path.exists():
                    shutil.rmtree(path)
            run(log, "cmake", "--install", str(build), "--config", config,
                "--prefix", str(staged))
            staged.rename(prefix)
            if args.rust:
                env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
                           CARGO_TARGET_DIR=str(work / "cargo"))
                cargo = ["cargo", "test", "--manifest-path",
                         str(ROOT / "bindings/rust/Cargo.toml"), "--workspace",
                         "--features", "malvado/assets"]
                if config == "Release":
                    cargo.append("--release")
                run(log, *cargo, env=env)

        print(f"PASS: {config}; evidence: {work / (config + '.log')}", flush=True)


if __name__ == "__main__":
    main()
