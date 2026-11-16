#!/usr/bin/env python3
"""Build the installed-SDK Rust NativeActivity verification fixture."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
import os
from pathlib import Path
import platform
import shlex
import subprocess
import sys
from test_android import ROOT, run


def build(log, work, installed, ndk, adb, serial, abi, config, token, lifecycle=0):
    target = {'arm64-v8a': 'aarch64-linux-android', 'x86_64': 'x86_64-linux-android'}[abi]
    game = work / 'Rust game'
    (game / 'src').mkdir(parents=True, exist_ok=True)
    (game / 'Cargo.toml').write_text('[package]\nname="android-rust-window"\nversion="0.0.0"\nedition="2024"\n'
        + '[lib]\ncrate-type=["staticlib"]\n[dependencies]\nmalvado={path='
        + json.dumps(str(installed / 'share/malvado/rust/malvado')) + ',features=["graphics","assets"]}\n')
    for name in ('lib.rs', 'keyboard.rs', 'touch.rs', 'assets.rs'):
        (game / 'src' / name).write_text((ROOT / 'tests/android-rust' / name).read_text())
    wrapper = work / 'probe runner'
    wrapper.write_text('#!/bin/sh\nexec ' + shlex.join([sys.executable, str(ROOT / 'tools/android_rust_runner.py')]) + ' "$@"\n')
    wrapper.chmod(0o755)
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(installed), MALVADO_ANDROID_NDK=str(ndk),
               MALVADO_ANDROID_RUNNER=str(wrapper), MALVADO_ANDROID_ADB=str(adb[0]), MALVADO_ANDROID_SERIAL=serial)
    host = 'darwin-x86_64' if platform.system() == 'Darwin' else 'linux-x86_64'
    clang_target = 'aarch64-linux-android' if abi == 'arm64-v8a' else target
    env['CARGO_TARGET_' + target.upper().replace('-', '_') + '_LINKER'] = str(ndk / f'toolchains/llvm/prebuilt/{host}/bin/{clang_target}28-clang')
    command = ['cargo', 'build', '--manifest-path', str(game / 'Cargo.toml'), '--target', target]
    if config == 'Release':
        command.append('--release')
    log.write(f'\n>>> {command!r}\n'); log.flush()
    subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=240, check=True)
    archive = game / 'target' / target / ('release' if config == 'Release' else 'debug') / 'libandroid_rust_window.a'
    output = work / 'Rust native'
    run(log, ['cmake', '-S', ROOT / 'tests/android-rust', '-B', output,
              f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
              f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28', f'-DCMAKE_BUILD_TYPE={config}',
              f'-DMALVADO_TEST_SDK={installed}', f'-DRUST_ARCHIVE={archive}', f'-DACCEPTANCE_TOKEN={token}', f'-DACCEPTANCE_LIFECYCLE={lifecycle}'])
    run(log, ['cmake', '--build', output, '--parallel', '2'])
    return output
