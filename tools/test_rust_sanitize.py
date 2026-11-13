#!/usr/bin/env python3
"""Test Rust callbacks against an ASan/UBSan-instrumented installed native core."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def run(*command, env=None):
    subprocess.run(command, cwd=ROOT, env=env, check=True)


def verify_leak_filter(compiler, flags, env, work):
    fixture = work / "lsan-scope"
    run(compiler, *flags.split(), "-g", "-fno-inline", "-O0",
        str(ROOT / "tests/lsan_scope.c"), "-o", str(fixture))
    for mode, should_pass in (("glx", True), ("owned", False)):
        result = subprocess.run([str(fixture), mode], env=env, text=True, capture_output=True)
        if (result.returncode == 0) != should_pass or (not should_pass and "LeakSanitizer" not in result.stderr):
            raise RuntimeError(f"leak filter fixture failed: {mode}\n{result.stdout}\n{result.stderr}")
    print("PASS: GLX filter preserves detection of unrelated native leaks", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assets", action="store_true", help="also verify native atlas services")
    parser.add_argument("--graphics", action="store_true", help="also verify window/input ownership (requires display)")
    args = parser.parse_args()
    variant = "graphics" if args.graphics else "assets" if args.assets else "core"
    work = ROOT / (f"build-rust-{variant}-sanitize" if variant != "core" else "build-rust-sanitize")
    compiler = shutil.which("clang")
    if compiler is None:
        raise SystemExit("Rust sanitizer checks require clang")
    flags = "-fsanitize=address,undefined -fno-omit-frame-pointer"
    prefix = work / "installed sdk"
    run("cmake", "-S", str(ROOT), "-B", str(work), "-DCMAKE_BUILD_TYPE=Debug",
        "-DMALVADO_BUILD_LUA=OFF", f"-DMALVADO_BUILD_RAYLIB={'ON' if args.graphics else 'OFF'}", f"-DMALVADO_BUILD_ASSETS={'ON' if args.assets else 'OFF'}",
        f"-DMALVADO_TEST_NULL_AUDIO={'ON' if args.graphics else 'OFF'}",
        "-DCMAKE_INSTALL_LIBDIR=lib", f"-DCMAKE_C_COMPILER={compiler}", f"-DCMAKE_C_FLAGS={flags}")
    run("cmake", "--build", str(work), "--target", "malvado_core", *(["malvado_assets"] if args.assets else []),
        *(["malvado_platform_raylib"] if args.graphics else []), "--parallel")
    run("cmake", "--install", str(work), "--prefix", str(prefix))
    runtime_dir = Path(subprocess.check_output([compiler, "-print-runtime-dir"], text=True).strip())
    rustflags = ["-C", f"linker={compiler}"]
    # rustc passes -nodefaultlibs, which suppresses Clang's sanitizer runtime.
    # ASan's combined runtime also supplies the UBSan handlers used by this core.
    if sys.platform == "darwin":
        runtime = runtime_dir / "libclang_rt.asan_osx_dynamic.dylib"
        rustflags += ["-C", f"link-arg={runtime}", "-C", f"link-arg=-Wl,-rpath,{runtime_dir}"]
    else:
        import platform
        if shutil.which("ld.lld") is None:
            raise SystemExit("Linux Rust sanitizer checks require the lld linker")
        rustflags += ["-C", "link-arg=-fuse-ld=lld"]
        runtime = runtime_dir / f"libclang_rt.asan-{platform.machine()}.a"
        rustflags += ["-C", "link-arg=-Wl,--whole-archive", "-C", f"link-arg={runtime}",
                      "-C", "link-arg=-Wl,--no-whole-archive", "-C", "link-arg=-ldl",
                      "-C", "link-arg=-lpthread", "-C", "link-arg=-lrt"]
    if not runtime.is_file():
        raise SystemExit(f"Clang sanitizer runtime is missing: {runtime}")
    env = dict(os.environ, CC=compiler, CFLAGS=flags, LDFLAGS=flags,
               MALVADO_SDK_PREFIX=str(prefix), CARGO_TARGET_DIR=str(work / "cargo"),
               CARGO_ENCODED_RUSTFLAGS="\x1f".join(rustflags),
               ASAN_OPTIONS="detect_leaks=0" if sys.platform == "darwin" else "detect_leaks=1",
               UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
    if args.graphics and sys.platform.startswith("linux"):
        env["ALSA_CONFIG_PATH"] = str(ROOT / "tests/alsa-null.conf")
        env["MALVADO_PIN_TEST_PROVIDERS"] = "1"
        env["LSAN_OPTIONS"] = f"suppressions={ROOT / 'tools/lsan-graphics.supp'}:print_suppressions=1"
        verify_leak_filter(compiler, flags, env, work)
    # Miri checks Rust future ownership separately; these native tests exercise
    # the real C scheduler, safe points, handles, and callback boundaries.
    features = (["malvado/assets"] if args.assets else []) + (["malvado/graphics"] if args.graphics else [])
    run("cargo", "test", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"),
        "--workspace", "--tests", *(["--features", ",".join(features)] if features else []), env=env)


if __name__ == "__main__":
    main()
