#!/usr/bin/env python3
"""Verify optional Rust assets against relocated installed native SDKs."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
import os
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from test_rust import ROOT, run

WORK = ROOT / "build-rust-assets"
MANIFEST = str(ROOT / "bindings/rust/Cargo.toml")


def rejected(prefix, expected):
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
               CARGO_TARGET_DIR=str(WORK / "rejections"))
    result = subprocess.run(["cargo", "check", "--manifest-path", MANIFEST,
                             "-p", "malvado", "--features", "assets"],
                            env=env, text=True, capture_output=True)
    if result.returncode == 0 or expected not in result.stdout + result.stderr:
        raise RuntimeError(f"expected {expected!r}\n{result.stdout}\n{result.stderr}")
    print(f"PASS: {expected}", flush=True)


def consumer(prefix, profile):
    game = WORK / "external assets game"
    (game / "src").mkdir(parents=True, exist_ok=True)
    sdk = prefix / "share/malvado/rust/malvado"
    (game / "Cargo.toml").write_text(
        '[package]\nname="assets-consumer"\nversion="0.0.0"\nedition="2024"\n'
        '[dependencies]\nmalvado={path=' + json.dumps(os.path.relpath(sdk, game))
        + ',features=["assets"]}\n')
    fixture = (ROOT / "tests/fixtures/atlas.json").read_text()
    aseprite = (ROOT / "tests/fixtures/aseprite/pingpong.json").read_text()
    fpg = (ROOT / "tests/fixtures/fpg/bennugd-8.fpg").read_bytes()
    (game / "src/main.rs").write_text(
        'use malvado::{atlas::Atlas, Error};\n'
        'fn main() -> Result<(), Error> {\n'
        ' let atlas = Atlas::parse(' + json.dumps(fixture) + ', 8, 8)?;\n'
        ' assert_eq!(atlas.sprite_named("green")?.unwrap().id, 42);\n'
        ' assert_eq!(atlas.image(), "red.png");\n'
        ' let imported = Atlas::parse_aseprite(' + json.dumps(aseprite) + ', 8, 8)?;\n'
        ' let clip = imported.animation_named("bounce")?.unwrap();\n'
        ' assert_eq!(clip.repeats(), Some(3));\n'
        ' let animation = clip.to_animation(true)?; drop(imported);\n'
        ' assert_eq!(animation.sample(600_000).graph_id, 2);\n'
        ' let library = malvado::fpg::Fpg::parse(&' + str(list(fpg)) + ')?;\n'
        ' assert_eq!(library.sprite(7).unwrap().pixels, &[0,1,1,0]);\n'
        ' assert_eq!(library.point(7,0).unwrap().x, 1);\n'
        ' assert_eq!(library.rgba(7,true)?[7], 255);\n'
        ' assert!(library.png(7,true)?.starts_with(b"\\x89PNG\\r\\n\\x1a\\n"));\n'
        ' let manifest = malvado::fpg::Manifest::parse(&library.json(true)?)?;\n'
        ' drop(library); assert!(manifest.transparent_zero());\n'
        ' assert_eq!(manifest.sprite(7).unwrap().width, 2);\n'
        ' Ok(())\n}\n')
    run("cargo", "run", "--offline", *(["--release"] if profile == "Release" else []), cwd=game,
        env=dict(os.environ, MALVADO_SDK_PREFIX=str(prefix)), timeout=60)


def main():
    for profile in ("Debug", "Release"):
        build = WORK / profile
        staged = build / "staged"
        prefix = build / "relocated assets sdk"
        run("cmake", "-S", str(ROOT), "-B", str(build),
            f"-DCMAKE_BUILD_TYPE={profile}", "-DMALVADO_BUILD_LUA=OFF",
            "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=ON",
            "-DCMAKE_INSTALL_LIBDIR=lib")
        run("cmake", "--build", str(build), "--parallel")
        run("cmake", "--install", str(build), "--prefix", str(staged))
        if prefix.exists():
            shutil.rmtree(prefix)
        staged.rename(prefix)
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
                   CARGO_TARGET_DIR=str(WORK / "target"))
        args = ["cargo", "test", "--manifest-path", MANIFEST,
                "--workspace", "--features", "malvado/assets"]
        if profile == "Release":
            args.append("--release")
        run(*args, env=env)
        run("cargo", "clippy", "--manifest-path", MANIFEST, "--workspace",
            "--all-targets", "--features", "malvado/assets", "--", "-D", "warnings", env=env)
        run("ctest", "--test-dir", str(build), "-C", profile, "--output-on-failure")
        consumer(prefix, profile)
    # A core-only SDK remains usable by default and rejects explicit assets.
    core = WORK / "core-only"
    run("cmake", "-S", str(ROOT), "-B", str(core), "-DMALVADO_BUILD_LUA=OFF",
        "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib", "-DFETCHCONTENT_FULLY_DISCONNECTED=ON")
    run("cmake", "--build", str(core), "--target", "malvado_core", "--parallel")
    run("cmake", "--install", str(core), "--prefix", str(core / "sdk"))
    rejected(core / "sdk", "Malvado_FOUND to FALSE")
    drift = WORK / "drift sdk"
    if drift.exists():
        shutil.rmtree(drift)
    shutil.copytree(prefix, drift)
    header = drift / "include/malvado/atlas.h"
    header.write_text(header.read_text().replace("int pivot_x, pivot_y;", "long long pivot_x, pivot_y;"))
    rejected(drift, "assets ABI layout/value mismatch")
    run("cargo", "fmt", "--manifest-path", MANIFEST, "--all", "--", "--check")


if __name__ == "__main__":
    main()
