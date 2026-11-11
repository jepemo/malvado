#!/usr/bin/env python3
"""Verify Rust against native Debug and Release SDKs without source-tree linking."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import os
import json
from pathlib import Path
import shutil
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "build-rust"


def run(*args, env=None, cwd=ROOT, timeout=None):
    subprocess.run(args, cwd=cwd, env=env, timeout=timeout, check=True)


def reject_sdk(label, prefix, expected):
    env = dict(os.environ, CARGO_TARGET_DIR=str(WORK / "diagnostics"))
    env.pop("MALVADO_SDK_PREFIX", None)
    if prefix is not None:
        env["MALVADO_SDK_PREFIX"] = str(prefix)
    result = subprocess.run(["cargo", "check", "--manifest-path",
                             str(ROOT / "bindings/rust/Cargo.toml"), "-p", "malvado-sys"],
                            cwd=ROOT, env=env, text=True, capture_output=True)
    if result.returncode == 0 or expected not in result.stdout + result.stderr:
        raise RuntimeError(f"{label}: expected SDK rejection {expected!r}\n{result.stdout}\n{result.stderr}")
    print(f"PASS: {label}", flush=True)


def diagnostics(prefix):
    reject_sdk("missing prefix", None, "set MALVADO_SDK_PREFIX")
    reject_sdk("nonexistent prefix", WORK / "does not exist", "MALVADO_SDK_PREFIX does not exist")
    empty = WORK / "empty sdk"
    empty.mkdir(exist_ok=True)
    reject_sdk("incomplete SDK", empty, "MalvadoConfig.cmake")
    fake = WORK / "incompatible sdk"
    if fake.exists():
        shutil.rmtree(fake)
    shutil.copytree(prefix, fake)
    source = fake / "version.c"
    archive = fake / "lib/libmalvado_core.a"
    compiler = shlex.split(os.environ.get("CC", "cc"))
    for label, abi, release, expected in (
            ("ABI mismatch", 999, None, "incompatible Malvado native ABI revision"),
            ("release mismatch", 1, "wrong-release", "incompatible Malvado native release")):
        # If ABI checking calls the release query too early, this fixture aborts.
        body = 'abort();' if release is None else f'return "{release}";'
        source.write_text('#include <stdint.h>\n#include <stdlib.h>\n'
                          f'uint32_t mv_abi_version(void) {{ return {abi}; }}\n'
                          f'const char *mv_version_string(void) {{ {body} }}\n')
        run(*compiler, "-c", str(source), "-o", str(fake / "version.o"))
        archive.unlink()
        run("ar", "rcs", str(archive), str(fake / "version.o"))
        reject_sdk(label, fake, expected)
    drift = WORK / "header drift sdk"
    if drift.exists():
        shutil.rmtree(drift)
    shutil.copytree(prefix, drift)
    header = drift / "include/malvado/malvado.h"
    original = header.read_text()
    header.write_text(original.replace("unsigned phase;", "unsigned phase; unsigned incompatible;"))
    reject_sdk("layout mismatch", drift, "native ABI layout/value mismatch")
    header.write_text(original.replace("MvResult mv_process_tick(", "void mv_process_tick("))
    reject_sdk("signature mismatch", drift, "mv_process_tick signature")
    dynamic = WORK / "dynamic sdk"
    config = dynamic / "lib/cmake/Malvado"
    config.mkdir(parents=True, exist_ok=True)
    (config / "MalvadoConfig.cmake").write_text(
        'add_library(Malvado::core SHARED IMPORTED)\nset(Malvado_core_FOUND TRUE)\n')
    reject_sdk("dynamic-only SDK", dynamic, "requires a static Malvado core SDK")


def external_consumer(prefix):
    sdk = prefix / "share/malvado/rust"
    assert (sdk / "malvado-sys/Cargo.toml").is_file()
    game = WORK / "external game"
    (game / "src").mkdir(parents=True, exist_ok=True)
    (game / "Cargo.toml").write_text(
        '[package]\nname = "sdk-consumer"\nversion = "0.0.0"\nedition = "2024"\n'
        '[dependencies]\nmalvado = { path = '
        + json.dumps(os.path.relpath(sdk / 'malvado', game)) + ' }\n')
    (game / "src/main.rs").write_text(
        'use malvado::{Engine, Error, SpawnOptions};\n'
        'use malvado::collision::{Rect, Shape, Transform, World};\n'
        'fn main() -> Result<(), Error> {\n'
        ' let geometry = malvado::graphic::Geometry::rectangle(20.0, 10.0, malvado::graphic::Pivot::TopLeft)?;\n'
        ' let _shape = geometry.collider(); let _circle = malvado::graphic::Geometry::circle(5.0)?;\n'
        '    let mut camera = malvado::camera::Camera::new(0.0, 0.0, None)?;\n'
        '    camera.center((200.0, 150.0), (800, 600))?;\n'
        '    assert_eq!(camera.project((210.0, 160.0), (0.0, 0.0))?, (410.0, 310.0));\n'
        '    let animation = malvado::animation::Animation::new(&[malvado::animation::Frame { graph_id: 1, duration_us: 100_000 }], true)?;\n'
        '    let mut playback = animation.playback(); playback.advance(60)?;\n'
        '    assert_eq!(playback.sample().graph_id, 1);\n'
        '    let mut engine = Engine::new()?;\n'
        '    engine.spawn(SpawnOptions::default(), |p| async move { p.frame().await; Ok(()) })?;\n'
        '    engine.tick()?; engine.tick()?;\n'
        '    assert_eq!(engine.process_count(), 0);\n'
        '    let mut world = World::new()?;\n'
        '    world.add_rect(Rect { x: 10.0, y: 0.0, width: 10.0, height: 10.0 })?;\n'
        '    let mut position = Transform { y: 5.0, ..Default::default() };\n'
        '    world.move_by(Shape::circle(1.0)?, &mut position, 100.0, 0.0)?;\n'
        '    assert_eq!(position.x, 9.0);\n'
        '    Ok(())\n}\n')
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix))
    run("cargo", "run", "--offline", env=env, cwd=game)


def main():
    native_env = dict(os.environ)
    native_env["PATH"] = os.pathsep.join(
        entry for entry in native_env["PATH"].split(os.pathsep)
        if not (Path(entry) / "cargo").is_file())
    assert shutil.which("cargo", path=native_env["PATH"]) is None
    for config in ("Debug", "Release"):
        build = WORK / config
        prefix = build / "relocated sdk"
        staged = build / "staged"
        run("cmake", "-S", str(ROOT), "-B", str(build),
            f"-DCMAKE_BUILD_TYPE={config}", "-DMALVADO_BUILD_LUA=OFF",
            "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=OFF",
            "-DCMAKE_INSTALL_LIBDIR=lib", "-DFETCHCONTENT_FULLY_DISCONNECTED=ON", env=native_env)
        run("cmake", "--build", str(build), "--parallel", env=native_env)
        run("cmake", "--install", str(build), "--prefix", str(staged), env=native_env)
        if prefix.exists():
            shutil.rmtree(prefix)
        staged.rename(prefix)
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix))
        cargo = ["cargo", "test", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"), "--workspace"]
        if config == "Release":
            cargo.append("--release")
        run(*cargo, env=env)
        for name in ("counter", "world"):
            example = ["cargo", "run", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"),
                       "-p", "malvado", "--example", name]
            if config == "Release":
                example.append("--release")
            run(*example, env=env, timeout=30)
        run("cargo", "clippy", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"),
            "--workspace", "--all-targets", "--", "-D", "warnings", env=env)
    diagnostics(prefix)
    external_consumer(prefix)
    run("cargo", "fmt", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"), "--all", "--", "--check")


if __name__ == "__main__":
    main()
