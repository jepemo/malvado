#!/usr/bin/env python3
"""Verify Rust window services against relocated graphics SDKs (needs a display)."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from test_rust import ROOT, run

WORK = ROOT / "build-rust-graphics"
MANIFEST = str(ROOT / "bindings/rust/Cargo.toml")


def consumer(prefix, profile):
    game = WORK / "external graphics game"
    (game / "src").mkdir(parents=True, exist_ok=True)
    sdk = prefix / "share/malvado/rust/malvado"
    (game / "Cargo.toml").write_text(
        '[package]\nname="graphics-consumer"\nversion="0.0.0"\nedition="2024"\n'
        '[dependencies]\nmalvado={path=' + json.dumps(os.path.relpath(sdk, game))
        + ',features=["graphics"]}\n')
    (game / "src/main.rs").write_text(
        'use malvado::{Error, window::{Window, Color}};\n'
        'fn main() -> Result<(), Error> {\n'
        ' let window = Window::open(160, 120, "Installed Rust window")?;\n'
        ' assert_eq!(window.size(), (160,120));\n'
        ' drop(window.frame(Color {r:10,g:20,b:30,a:255})?);\n'
        ' Ok(())\n}\n')
    run("cargo", "run", "--offline", *(["--release"] if profile == "Release" else []), cwd=game,
        env=dict(os.environ, MALVADO_SDK_PREFIX=str(prefix)), timeout=60)


def main():
    for profile in ("Debug", "Release"):
        build = WORK / profile / "native"
        staged = WORK / profile / "staged"
        prefix = WORK / profile / "relocated graphics sdk"
        args = ["cmake", "-S", str(ROOT), "-B", str(build),
                f"-DCMAKE_BUILD_TYPE={profile}", "-DMALVADO_BUILD_LUA=ON",
                "-DMALVADO_BUILD_RAYLIB=ON", "-DMALVADO_VISUAL_TESTS=ON",
                "-DCMAKE_INSTALL_LIBDIR=lib"]
        # Reuse verified FetchContent sources when a host suite ran first.
        for name in ("lua", "raylib", "cjson"):
            source = ROOT / "build-visual/_deps" / f"{name}-src"
            if source.is_dir():
                args.append(f"-DFETCHCONTENT_SOURCE_DIR_{name.upper()}={source}")
        run(*args)
        run("cmake", "--build", str(build), "--parallel")
        run("cmake", "--install", str(build), "--prefix", str(staged))
        if prefix.exists():
            shutil.rmtree(prefix)
        staged.rename(prefix)
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
                   CARGO_TARGET_DIR=str(WORK / "target"))
        cargo = ["cargo", "test", "--manifest-path", MANIFEST, "--workspace", "--all-features"]
        if profile == "Release":
            cargo.append("--release")
        run(*cargo, env=env, timeout=180)
        run("cargo", "clippy", "--manifest-path", MANIFEST, "--workspace",
            "--all-targets", "--all-features", "--", "-D", "warnings", env=env)
        consumer(prefix, profile)
    # Feature selection must fail clearly for a core-only installed SDK.
    core = ROOT / "build-rust/Release/relocated sdk"
    if core.is_dir():
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(core),
                   CARGO_TARGET_DIR=str(WORK / "rejections"))
        result = subprocess.run(["cargo", "check", "--manifest-path", MANIFEST,
                                 "-p", "malvado", "--features", "graphics"],
                                env=env, text=True, capture_output=True)
        if result.returncode == 0 or "Malvado_FOUND to FALSE" not in result.stdout + result.stderr:
            raise RuntimeError("core-only SDK did not reject graphics feature\n" + result.stdout + result.stderr)
    run("cargo", "fmt", "--manifest-path", MANIFEST, "--all", "--", "--check")


if __name__ == "__main__":
    main()
