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


def diagnostics(log, work, prefix, linker, target, env, config):
    fake = work / config / 'incompatible sdk'
    if fake.exists():
        shutil.rmtree(fake)
    shutil.copytree(prefix, fake)
    archive = fake / 'lib/libmalvado_core.a'
    source = fake / 'version.c'
    header = (ROOT / 'src/main.c').read_text().split('*/', 1)[0] + '*/\n'
    cases = [('previous ABI', 1, None, 'incompatible Malvado native ABI revision'),
             ('ABI mismatch', 999, None, 'incompatible Malvado native ABI revision'),
             ('release mismatch', 2, 'wrong-release', 'incompatible Malvado native release')]
    for label, abi, release, expected in cases:
        body = 'abort();' if release is None else f'return "{release}";'
        source.write_text(header + '#include <stdint.h>\n#include <stdlib.h>\n'
                          + f'uint32_t mv_abi_version(void) {{ return {abi}; }}\n'
                          + f'const char *mv_version_string(void) {{ {body} }}\n')
        run(log, [linker, '-c', source, '-o', fake / 'version.o'])
        archive.unlink()
        run(log, [linker.parent / 'llvm-ar', 'rcs', archive, fake / 'version.o'])
        rejected_env = dict(env, MALVADO_SDK_PREFIX=str(fake))
        result = subprocess.run(['cargo', 'check', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                                 '-p', 'malvado-sys', '--target', target], env=rejected_env,
                                cwd=ROOT, capture_output=True, text=True, timeout=120)
        log.write(result.stdout + result.stderr); log.flush()
        if result.returncode == 0 or expected not in result.stdout + result.stderr:
            raise RuntimeError(f'{label}: expected target SDK rejection: {expected}')
        print(f'PASS: Android Rust {config}: {label}', flush=True)
    for feature in ('graphics',):
        result = subprocess.run(['cargo', 'check', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                                 '-p', 'malvado-sys', '--target', target, '--features', feature],
                                env=env, cwd=ROOT, capture_output=True, text=True, timeout=120)
        log.write(result.stdout + result.stderr); log.flush()
        expected = f'Rust Android {feature} is not yet supported'
        if result.returncode == 0 or expected not in result.stdout + result.stderr:
            raise RuntimeError(f'Expected explicit unsupported-feature rejection: {feature}')
        print(f'PASS: Android Rust {config}: unavailable {feature}', flush=True)
    return len(cases) + 1


def asset_diagnostics(log, work, prefix, ndk, abi, target, env, config):
    drift = work / config / 'asset layout drift sdk'
    if drift.exists():
        shutil.rmtree(drift)
    shutil.copytree(prefix, drift)
    header = drift / 'include/malvado/atlas.h'
    text = header.read_text()
    changed = text.replace('int pivot_x, pivot_y;', 'long long pivot_x, pivot_y;')
    if changed == text:
        raise RuntimeError('Asset drift fixture did not change the native header')
    header.write_text(changed)
    core = work / config / 'core only'
    run(log, ['cmake', '-S', ROOT, '-B', core,
              f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
              f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
              f'-DCMAKE_BUILD_TYPE={config}', '-DCMAKE_INSTALL_LIBDIR=lib',
              '-DMALVADO_BUILD_LUA=OFF', '-DMALVADO_BUILD_RAYLIB=OFF',
              '-DMALVADO_BUILD_ASSETS=OFF', '-DFETCHCONTENT_FULLY_DISCONNECTED=ON'])
    run(log, ['cmake', '--build', core, '--parallel', '8'])
    run(log, ['cmake', '--install', core, '--prefix', core / 'sdk'])
    for label, sdk, expected in [('asset layout drift', drift, 'assets ABI layout/value mismatch'),
                                 ('missing asset component', core / 'sdk', 'Malvado_FOUND to FALSE')]:
        result = subprocess.run(['cargo', 'check', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                                 '-p', 'malvado-sys', '--target', target, '--features', 'assets'],
                                env=dict(env, MALVADO_SDK_PREFIX=str(sdk)), cwd=ROOT,
                                capture_output=True, text=True, timeout=120)
        log.write(result.stdout + result.stderr); log.flush()
        if result.returncode == 0 or expected not in result.stdout + result.stderr:
            raise RuntimeError(f'{label}: expected asset SDK rejection: {expected}')
        print(f'PASS: Android Rust {config}: {label}', flush=True)
    # Optional assets must not prevent a core-only target SDK from being used.
    subprocess.run(['cargo', 'check', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                             '-p', 'malvado', '--target', target],
                            env=dict(env, MALVADO_SDK_PREFIX=str(core / 'sdk')), cwd=ROOT,
                            stdout=log, stderr=subprocess.STDOUT, timeout=120, check=True)
    return 2


def installed_consumer(log, work, prefix, target, env, config, wrapper):
    game = work / config / 'external assets game'
    (game / 'src').mkdir(parents=True, exist_ok=True)
    crate = prefix / 'share/malvado/rust/malvado'
    (game / 'Cargo.toml').write_text('[package]\nname="android-assets-consumer"\nversion="0.0.0"\nedition="2024"\n'
        + '[dependencies]\nmalvado={path=' + json.dumps(str(crate)) + ',features=["assets"]}\n')
    (game / 'src/main.rs').write_text('''// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::asset::{Source, Library, Format, Metadata};
fn main() {
    let root = std::path::PathBuf::from(std::env::var_os("MALVADO_TEST_ROOT").unwrap());
    let source = Source::open(root.join("tests/fixtures/atlas.json")).unwrap();
    assert_eq!(source.format(), Format::Atlas);
    let library = Library::from_source(source, None).unwrap();
    let Metadata::Atlas(atlas) = library.into_metadata() else { panic!("expected atlas") };
    assert_eq!(atlas.sprite(42).unwrap().pivot, (2, 4));
    let fpg = Library::open(root.join("tests/fixtures/fpg/bennugd-8.fpg"), None).unwrap();
    assert_eq!(fpg.format(), Format::Fpg);
    assert!(Source::open(root.join("missing.png")).is_err());
    println!("installed Android Rust assets consumer passed");
}
''')
    command = ['cargo', 'run', '--offline', '--manifest-path', str(game / 'Cargo.toml'),
               '--target', target, '--config', f'target.{target}.runner={json.dumps([str(wrapper)])}']
    if config == 'Release':
        command.append('--release')
    subprocess.run(command, env=env, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)


def verify(sdk, serial, work, config, *, assets=False):
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
                  '-DMALVADO_BUILD_LUA=OFF', '-DMALVADO_BUILD_RAYLIB=OFF', f'-DMALVADO_BUILD_ASSETS={"ON" if assets else "OFF"}'])
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
        if assets:
            inputs = work / 'test inputs'
            if inputs.exists():
                shutil.rmtree(inputs)
            names = subprocess.check_output(['git', 'ls-files', '-z', 'tests/fixtures', 'examples/aseprite'], cwd=ROOT).decode().split('\0')
            for name in filter(None, names):
                source = ROOT / name
                if source.is_symlink():
                    raise RuntimeError('Test input cannot be a symlink: ' + name)
                destination = inputs / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, destination)
            env['MALVADO_ANDROID_TEST_INPUTS'] = str(inputs)
        key = target.upper().replace('-', '_')
        env[f'CARGO_TARGET_{key}_LINKER'] = str(linker)
        command = ['cargo', 'test', '--manifest-path', str(ROOT / 'bindings/rust/Cargo.toml'),
                   '-p', 'malvado', '--target', target, '--lib', '--tests',
                   '--config', f'target.{target}.runner={json.dumps([str(wrapper)])}']
        if assets:
            command.extend(['--features', 'assets'])
        if config == 'Release':
            command.append('--release')
        log.write(f'\n>>> {command!r}\n'); log.flush()
        subprocess.run(command, env=env, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
        if assets:
            installed_consumer(log, work, prefix, target, env, config, wrapper)
        rejected = diagnostics(log, work, prefix, linker, target, env, config)
        asset_rejections = asset_diagnostics(log, work, prefix, ndk, abi, target, env, config) if assets else 0
        result_file.write_text(json.dumps(dict(facts, configuration=config, target=target, assets=assets, asset_rejections=asset_rejections, installed_assets_consumer='passed' if assets else 'not requested', sdk_rejections=rejected,
            rust='headless core/assets tests passed' if assets else 'headless core tests passed', source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT).decode().strip(),
            dirty=bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT))), indent=2) + '\n')
    print(f'PASS: Android Rust {target} {config}', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--work', type=Path, default=ROOT / 'build-android-rust')
    parser.add_argument('--assets', action='store_true', help='Verify optional asset services and staged file fixtures')
    parser.add_argument('--config', choices=('Debug', 'Release'), action='append')
    args = parser.parse_args()
    for config in args.config or ('Debug', 'Release'):
        verify(args.sdk.resolve(), args.serial, args.work.resolve(), config, assets=args.assets)


if __name__ == '__main__':
    main()
