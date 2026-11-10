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
        'use malvado::{Error, drawing::Rectangle, window::{Window, Color}};\n'
        'fn main() -> Result<(), Error> {\n'
        ' let window = Window::open(160, 120, "Installed Rust window")?;\n'
        ' assert_eq!(window.size(), (160,120));\n'
        ' window.input_tick();\n'
        ' assert!(!window.key_pressed(malvado::window::Key::Space));\n'
        ' assert!(!window.key_released(malvado::window::Key::Space));\n'
        ' let frame = window.frame(Color {r:10,g:20,b:30,a:255})?;\n'
        ' frame.with_clip(Rectangle {x:0,y:0,width:80,height:60}, |frame| {\n'
        '     frame.rectangle(Rectangle {x:10,y:10,width:20,height:20}, true, Color {r:255,g:0,b:0,a:255})\n'
        ' })?;\n'
        ' drop(frame);\n'
        ' Ok(())\n}\n')
    run("cargo", "run", "--offline", *(["--release"] if profile == "Release" else []), cwd=game,
        env=dict(os.environ, MALVADO_SDK_PREFIX=str(prefix)), timeout=60)


def capture(command, env=None):
    result = subprocess.run(command, env=env, cwd=ROOT, text=True, capture_output=True, timeout=180)
    print(result.stdout, end="", flush=True)
    print(result.stderr, end="", file=sys.stderr, flush=True)
    result.check_returncode()
    return result.stdout


def parity(output, prefix):
    expected = (ROOT / "bindings/rust/malvado/tests/fixtures/keyboard.txt").read_text().splitlines() * 2
    actual = [line.removeprefix(prefix) for line in output.splitlines() if line.startswith(prefix)]
    if actual != expected:
        raise RuntimeError(f"{prefix} keyboard trace mismatch: {actual!r}")


def rejected(prefix, expected):
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
               CARGO_TARGET_DIR=str(WORK / "rejections"))
    result = subprocess.run(["cargo", "check", "--manifest-path", MANIFEST,
                             "-p", "malvado", "--features", "graphics"],
                            env=env, text=True, capture_output=True)
    if result.returncode == 0 or expected not in result.stdout + result.stderr:
        raise RuntimeError(f"expected graphics SDK rejection {expected!r}\n{result.stdout}\n{result.stderr}")
    print(f"PASS: {expected}", flush=True)


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
        output = capture(cargo, env=env)
        parity(output, "rust-key: ")
        output = capture([str(build / "key-edges-visual-test"), str(ROOT / "tests/key_edges_visual.lua")])
        parity(output, "native-key: ")
        parity(output, "lua-key: ")
        if sys.platform.startswith("linux"):
            if shutil.which("xdotool") is None:
                raise RuntimeError("Linux keyboard injection requires xdotool")
            command = ["cargo", "test", "--manifest-path", MANIFEST, "-p", "malvado",
                       "--all-features", "--test", "keyboard", "--no-run", "--message-format=json"]
            if profile == "Release": command.append("--release")
            artifacts = capture(command, env=env)
            executable = next(json.loads(line)["executable"] for line in artifacts.splitlines()
                              if line.startswith("{") and json.loads(line).get("executable"))
            run("sh", str(ROOT / "tests/keys_visual.sh"), "Malvado Rust keys", executable,
                "--physical", env=env, timeout=30)
        run("cargo", "clippy", "--manifest-path", MANIFEST, "--workspace",
            "--all-targets", "--all-features", "--", "-D", "warnings", env=env)
        consumer(prefix, profile)
    # Verify feature discovery even when this task runs before the core suite.
    core = WORK / "core-only"
    run("cmake", "-S", str(ROOT), "-B", str(core), "-DMALVADO_BUILD_LUA=OFF",
        "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib", "-DFETCHCONTENT_FULLY_DISCONNECTED=ON")
    run("cmake", "--build", str(core), "--target", "malvado_core", "--parallel")
    run("cmake", "--install", str(core), "--prefix", str(core / "sdk"))
    rejected(core / "sdk", "Malvado_FOUND to FALSE")
    drift = WORK / "drift sdk"
    if drift.exists(): shutil.rmtree(drift)
    shutil.copytree(prefix, drift)
    header = drift / "include/malvado/platform.h"
    header.write_text(header.read_text().replace("unsigned char r, g, b, a;", "unsigned int r, g, b, a;"))
    rejected(drift, "graphics ABI layout/value mismatch")
    run("cargo", "fmt", "--manifest-path", MANIFEST, "--all", "--", "--check")


if __name__ == "__main__":
    main()
