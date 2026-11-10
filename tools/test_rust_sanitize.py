#!/usr/bin/env python3
"""Test Rust callbacks against an ASan/UBSan-instrumented installed native core."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "build-rust-sanitize"


def run(*command, env=None):
    subprocess.run(command, cwd=ROOT, env=env, check=True)


def main():
    compiler = shutil.which("clang")
    if compiler is None:
        raise SystemExit("Rust sanitizer checks require clang")
    flags = "-fsanitize=address,undefined -fno-omit-frame-pointer"
    prefix = WORK / "installed sdk"
    run("cmake", "-S", str(ROOT), "-B", str(WORK), "-DCMAKE_BUILD_TYPE=Debug",
        "-DMALVADO_BUILD_LUA=OFF", "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib", f"-DCMAKE_C_COMPILER={compiler}", f"-DCMAKE_C_FLAGS={flags}")
    run("cmake", "--build", str(WORK), "--target", "malvado_core", "--parallel")
    run("cmake", "--install", str(WORK), "--prefix", str(prefix))
    runtime_dir = Path(subprocess.check_output([compiler, "-print-runtime-dir"], text=True).strip())
    rustflags = ["-C", f"linker={compiler}"]
    # rustc passes -nodefaultlibs, which suppresses Clang's sanitizer runtime.
    # ASan's combined runtime also supplies the UBSan handlers used by this core.
    if sys.platform == "darwin":
        runtime = runtime_dir / "libclang_rt.asan_osx_dynamic.dylib"
        rustflags += ["-C", f"link-arg={runtime}", "-C", f"link-arg=-Wl,-rpath,{runtime_dir}"]
    else:
        import platform
        runtime = runtime_dir / f"libclang_rt.asan-{platform.machine()}.a"
        rustflags += ["-C", "link-arg=-Wl,--whole-archive", "-C", f"link-arg={runtime}",
                      "-C", "link-arg=-Wl,--no-whole-archive", "-C", "link-arg=-ldl",
                      "-C", "link-arg=-lpthread", "-C", "link-arg=-lrt"]
    if not runtime.is_file():
        raise SystemExit(f"Clang sanitizer runtime is missing: {runtime}")
    env = dict(os.environ, CC=compiler, CFLAGS=flags, LDFLAGS=flags,
               MALVADO_SDK_PREFIX=str(prefix), CARGO_TARGET_DIR=str(WORK / "cargo"),
               CARGO_ENCODED_RUSTFLAGS="\x1f".join(rustflags),
               ASAN_OPTIONS="detect_leaks=0" if sys.platform == "darwin" else "detect_leaks=1",
               UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
    # Miri checks Rust future ownership separately; these native tests exercise
    # the real C scheduler, safe points, handles, and callback boundaries.
    run("cargo", "test", "--manifest-path", str(ROOT / "bindings/rust/Cargo.toml"),
        "--workspace", "--tests", env=env)


if __name__ == "__main__":
    main()
