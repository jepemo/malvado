#!/usr/bin/env python3
"""Build and test a native headless SDK in Debug and Release."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parents[1]


def run(log, *args, env=None, timeout=600):
    print(f">>> {args!r}", flush=True)
    log.write(f"\n>>> {args!r}\n")
    log.flush()
    try:
        subprocess.run(args, cwd=ROOT, env=env, stdout=log,
                       stderr=subprocess.STDOUT, check=True, timeout=timeout)
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
        log.flush()
        print(f"Command failed; full evidence: {log.name}", flush=True)
        print("\n".join(Path(log.name).read_text(encoding="utf-8", errors="replace").splitlines()[-60:]),
              flush=True)
        raise


def graphics_runner(graphics):
    if not graphics or os.name != "nt":
        return []
    command = [sys.executable, str(ROOT / "tools/windows_graphics.py"), "run"]
    return ["--config", "target.x86_64-pc-windows-msvc.runner=" + json.dumps(command)]


def verify_rust_sdk(log, work, prefix, config, graphics=False):
    path_tests = work / ("sdk-path-tests.exe" if os.name == "nt" else "sdk-path-tests")
    run(log, "rustc", "--test", str(ROOT / "bindings/rust/malvado-sys/build.rs"),
        "-o", str(path_tests))
    run(log, str(path_tests))
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
               CARGO_TARGET_DIR=str(work / "cargo"))
    manifest = str(ROOT / "bindings/rust/Cargo.toml")
    cargo = ["cargo", "test", "--manifest-path", manifest, "--workspace", "--no-fail-fast",
             "--features", "malvado/assets,malvado/graphics" if graphics else "malvado/assets"]
    if config == "Release":
        cargo.append("--release")
    run(log, *cargo, *graphics_runner(graphics), env=env)

    # Exercise the crates shipped in the SDK, not the source-tree dependency.
    game = work / "external Rust game"
    (game / "src").mkdir(parents=True, exist_ok=True)
    dependency = prefix / "share/malvado/rust/malvado"
    (game / "Cargo.toml").write_text(
        '[package]\nname="platform-consumer"\nversion="0.0.0"\nedition="2024"\n'
        '[dependencies]\nmalvado={path=' + json.dumps(str(dependency))
        + (',features=["assets","graphics"]}\n' if graphics else ',features=["assets"]}\n'), encoding="utf-8")
    window_probe = (
        ' let window = malvado::window::Window::open(160,120,"Installed SDK")?;\n'
        ' let frame = window.frame(malvado::window::Color::default())?;\n'
        ' drop(frame); drop(window);\n'
    ) if graphics else ""
    (game / "src/main.rs").write_text(
        'use malvado::{Game, SpawnOptions};\n'
        'fn main() -> Result<(), malvado::Error> {\n' + window_probe +
        ' let mut game = Game::new()?;\n'
        ' let process = game.spawn(SpawnOptions::default(), |p| async move {\n'
        '  p.frame().await; Ok(())\n'
        ' })?;\n'
        ' process.set_position(3.0,4.0)?;\n'
        ' assert_eq!(process.position()?, (3.0,4.0));\n'
        ' game.tick()?; game.tick()?;\n'
        ' assert_eq!(game.process_count(),0); Ok(())\n'
        '}\n', encoding="utf-8")
    command = ["cargo", "run", "--offline", "--manifest-path", str(game / "Cargo.toml")]
    if config == "Release":
        command.append("--release")
    run(log, *command, *graphics_runner(graphics), env=env)

    # These checks need neither Unix archive tools nor a replacement compiler.
    for prefix_value, expected in ((None, "set MALVADO_SDK_PREFIX"),
                                   (str(work / "missing sdk"), "MALVADO_SDK_PREFIX does not exist")):
        invalid = dict(env, CARGO_TARGET_DIR=str(work / "diagnostics"))
        invalid.pop("MALVADO_SDK_PREFIX", None)
        if prefix_value is not None:
            invalid["MALVADO_SDK_PREFIX"] = prefix_value
        result = subprocess.run(
            ["cargo", "check", "--manifest-path", manifest, "-p", "malvado-sys"],
            cwd=ROOT, env=invalid, text=True, capture_output=True, timeout=120)
        log.write(f"\n>>> Expect SDK rejection: {expected}\n")
        log.write(result.stdout + result.stderr)
        log.flush()
        if result.returncode == 0 or expected not in result.stdout + result.stderr:
            raise RuntimeError(f"SDK diagnostic missing: {expected}")
        log.write(f"PASS: {expected}\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work", type=Path, default=ROOT / "build-platform")
    parser.add_argument("--rust", action="store_true", help="Test Rust core and assets against relocated SDKs")
    parser.add_argument("--cli", action="store_true", help="Verify Windows CLI and produce fresh-runner players")
    parser.add_argument("--graphics", action="store_true", help="Test graphics with a display and paced silent audio")
    args = parser.parse_args()
    if args.cli and (not args.graphics or os.name != "nt"):
        parser.error("--cli requires native Windows and --graphics")
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    if args.graphics and os.name == "nt":
        from windows_graphics import prepare
        os.environ["MALVADO_MESA_DIR"] = str(prepare(work / "mesa"))
        os.environ["GALLIUM_DRIVER"] = "llvmpipe"
    for config in ("Debug", "Release"):
        build = work / config
        with (work / f"{config}.log").open("w", encoding="utf-8") as log:
            run(log, "cmake", "-S", str(ROOT), "-B", str(build),
                f"-DCMAKE_BUILD_TYPE={config}",
                "-DMALVADO_BUILD_RAYLIB=" + ("ON" if args.graphics else "OFF"),
                "-DMALVADO_VISUAL_TESTS=" + ("ON" if args.graphics else "OFF"),
                "-DMALVADO_TEST_NULL_AUDIO=" + ("ON" if args.graphics else "OFF"),
                "-DOPENGL_VERSION=OFF",
                "-DMALVADO_BUILD_LUA=ON", "-DMALVADO_BUILD_ASSETS=ON",
                "-DCMAKE_INSTALL_LIBDIR=lib",
                # Existing C fixtures use assert() to execute operations.
                "-DCMAKE_C_FLAGS_RELEASE=" + ("/O2 /UNDEBUG" if os.name == "nt" else "-O3 -UNDEBUG"))
            run(log, "cmake", "--build", str(build), "--config", config, "--parallel")
            run(log, "ctest", "--test-dir", str(build), "-C", config,
                "--output-on-failure", "--timeout", "60", "-E", "^visual_")
            if args.graphics:
                if os.name == "nt":
                    from windows_graphics import deploy
                    deploy(build / config)
                run(log, "ctest", "--test-dir", str(build), "-C", config,
                    "--output-on-failure", "--timeout", "60", "-R",
                    "^visual_(c_window|image_rgba|window_clip|window_scroll|sleep_freeze)$")
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
                verify_rust_sdk(log, work, prefix, config, args.graphics)
            if args.cli:
                run(log, sys.executable, str(ROOT / "tests/windows_cli.py"),
                    "--sdk", str(prefix), "--work", str(build / "CLI paths with spaces"),
                    "--players", str(work / "players" / config), timeout=900)

        print(f"PASS: {config}; evidence: {work / (config + '.log')}", flush=True)


if __name__ == "__main__":
    main()
