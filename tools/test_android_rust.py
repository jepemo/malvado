#!/usr/bin/env python3
"""Verify headless Rust against relocated Android SDKs on an API 34 emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import platform
import shlex
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from test_android import ROOT, preflight, run


def verify(sdk, serial, work, config):
    work.mkdir(parents=True, exist_ok=True)
    result_file = work / f'{config}-results.json'
    result_file.unlink(missing_ok=True)
    with (work / f'{config}.log').open('w') as log:
        ndk, adb, abi, facts = preflight(sdk, serial, log)
        target = {'arm64-v8a': 'aarch64-linux-android', 'x86_64': 'x86_64-linux-android'}[abi]
        build = work / config / 'native'
        prefix = work / config / 'relocated sdk'
        run(log, ['cmake', '-S', ROOT, '-B', build,
                  f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                  f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                  f'-DCMAKE_BUILD_TYPE={config}', '-DCMAKE_INSTALL_LIBDIR=lib',
                  '-DMALVADO_BUILD_LUA=OFF', '-DMALVADO_BUILD_RAYLIB=OFF', '-DMALVADO_BUILD_ASSETS=OFF'])
        run(log, ['cmake', '--build', build, '--parallel', '8'])
        staged = work / config / 'staged sdk'
        for directory in (staged, prefix):
            if directory.exists():
                shutil.rmtree(directory)
        run(log, ['cmake', '--install', build, '--prefix', staged])
        staged.rename(prefix)
        host = 'darwin-x86_64' if platform.system() == 'Darwin' else 'linux-x86_64'
        clang_target = 'aarch64-linux-android' if abi == 'arm64-v8a' else target
        linker = ndk / f'toolchains/llvm/prebuilt/{host}/bin/{clang_target}28-clang'
        wrapper = work / 'target runner'
        wrapper.write_text('#!/bin/sh\nexec ' + shlex.join([sys.executable, str(ROOT / 'tools/android_rust_runner.py')]) + ' "$@"\n')
        wrapper.chmod(0o755)
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix), MALVADO_ANDROID_NDK=str(ndk),
                   MALVADO_ANDROID_RUNNER=str(wrapper), MALVADO_ANDROID_ADB=str(adb[0]),
                   MALVADO_ANDROID_SERIAL=serial, CARGO_TARGET_DIR=str(work / 'cargo'))
        key = target.upper().replace('-', '_')
        env[f'CARGO_TARGET_{key}_LINKER'] = str(linker)
        command = ['cargo', 'test', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                   '-p', 'malvado', '--target', target, '--lib', '--tests',
                   '--config', f'target.{target}.runner={json.dumps([str(wrapper)])}']
        if config == 'Release':
            command.append('--release')
        log.write(f'\n>>> {command!r}\n'); log.flush()
        subprocess.run(command, env=env, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        result_file.write_text(json.dumps(dict(facts, configuration=config, target=target,
            rust='headless core tests passed', source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT).decode().strip(),
            dirty=bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT))), indent=2) + '\n')
    print(f'PASS: Android Rust {target} {config}', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--work', type=Path, default=ROOT / 'build-android-rust')
    parser.add_argument('--config', choices=('Debug', 'Release'), action='append')
    args = parser.parse_args()
    for config in args.config or ('Debug', 'Release'):
        verify(args.sdk.resolve(), args.serial, args.work.resolve(), config)


if __name__ == '__main__':
    main()
