#!/usr/bin/env python3
"""Build and run bounded C/Lua NativeActivity APK checks with the existing backend."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from pathlib import Path
import json
import os
import subprocess
import struct
import re
import shutil
import sys
import time
import uuid
import zipfile
sys.dont_write_bytecode = True
from test_android import ROOT, preflight, properties, run


def verify_java(output):
    version = re.search(r'^openjdk version "21\.0\.7"(?:\s|$)', output, re.MULTILINE)
    build = re.search(r'Temurin-21\.0\.7\+6(?:-LTS|[\s)])', output)
    if not version or not build:
        raise RuntimeError('Android APK verification requires Temurin 21.0.7+6; run mise install java')


def packaging_tools(sdk, log, facts):
    tools = sdk / 'build-tools/34.0.0'
    for tool in ('aapt', 'zipalign', 'apksigner'):
        if not (tools / tool).is_file():
            raise RuntimeError('Install Android build-tools;34.0.0')
    version = properties(tools / 'source.properties').get('Pkg.Revision')
    if version != '34.0.0':
        raise RuntimeError('Android APK verification requires Build Tools 34.0.0')
    platform = properties(sdk / 'platforms/android-34/source.properties')
    if platform.get('AndroidVersion.ApiLevel') != '34' or not (sdk / 'platforms/android-34/android.jar').is_file():
        raise RuntimeError('Android APK verification requires platform API 34')
    java = run(log, ['java', '-version'], timeout=15).stdout.strip()
    verify_java(java)
    facts.update(build_tools=version, platform_revision=platform.get('Pkg.Revision'),
                 platform_extension=platform.get('AndroidVersion.ExtensionLevel'),
                 java=java)
    return tools


def add_lua_assets(archive, entry, failure, *, keyboard=False):
    if entry not in ('explicit', 'declarative') or failure not in ('none', 'process', 'missing-image'):
        raise ValueError('Unsupported Lua APK fixture mode')
    fixture = ROOT / 'tests/android-lua'
    archive.write(fixture / (entry + '.lua'), 'assets/main.lua')
    archive.write(fixture / 'behavior.lua', 'assets/behavior.lua')
    if keyboard:
        archive.write(fixture / 'keyboard.lua', 'assets/keyboard.lua')
    archive.write(ROOT / 'tests/fixtures/atlas.json', 'assets/atlas.json')
    archive.writestr('assets/message.bin', bytes([0, 13, 10, 255]))
    if failure != 'missing-image':
        archive.write(ROOT / 'tests/fixtures/red.png', 'assets/red.png')


def add_rust_assets(archive, failure='none'):
    if failure not in ('none', 'process', 'missing-image'):
        raise ValueError('Unsupported Rust APK failure mode')
    archive.writestr('assets/message.bin', bytes([0, 13, 10, 255]))
    for name in (('atlas.json',) if failure == 'missing-image' else ('atlas.json', 'red.png')):
        archive.write(ROOT / 'tests/fixtures' / name, 'assets/' + name)


def add_audio_assets(archive, lua=False, rust=False):
    for name in ('silence.wav', 'silence.ogg', 'empty.wav'):
        archive.write(ROOT / 'tests/fixtures' / name, 'assets/' + name)
    if lua:
        archive.write(ROOT / 'tests/android-lua/audio.lua', 'assets/audio.lua')


def wait_marker(log, adb, marker, timeout=30):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        output = run(log, adb + ['logcat', '-d', '-s', 'MalvadoAcceptance:I', '*:S'], timeout=10).stdout
        if marker in output:
            return
        time.sleep(0.5)
    raise RuntimeError('Android acceptance marker missing: ' + marker)


def activity_is_background(state, package):
    resumed = [line for line in state.splitlines()
               if 'mResumedActivity:' in line or 'topResumedActivity=' in line]
    return bool(resumed) and all(package not in line for line in resumed)


def png_dimensions(header):
    if len(header) < 24 or header[:8] != b'\x89PNG\r\n\x1a\n' or header[8:12] != b'\x00\x00\x00\x0d' or header[12:16] != b'IHDR':
        raise ValueError('Invalid Android screenshot PNG header')
    width, height = struct.unpack('>II', header[16:24])
    if not (0 < width <= 16384 and 0 < height <= 16384):
        raise ValueError('Invalid Android screenshot dimensions')
    return width, height


def capture_screen(log, adb, destination):
    temporary = destination.with_suffix('.tmp')
    try:
        log.write(f"\n>>> Capture Android screenshot: {destination}\n")
        log.flush()
        try:
            with temporary.open('wb') as output:
                subprocess.run(adb + ['exec-out', 'screencap', '-p'], stdout=output,
                               stderr=subprocess.PIPE, check=True, timeout=15)
        except subprocess.SubprocessError as error:
            detail = getattr(error, 'stderr', None) or b''
            log.write(detail.decode(errors='replace') if isinstance(detail, bytes) else detail)
            log.flush()
            raise
        with temporary.open('rb') as image:
            dimensions = png_dimensions(image.read(24))
        temporary.replace(destination)
        return dimensions
    finally:
        temporary.unlink(missing_ok=True)


def lifecycle_actions(log, adb, package, token, mode, screenshots=None, lua=False, rust=False):
    wait_marker(log, adb, token + ' lifecycle ready')
    before = run(log, adb + ['shell', 'pidof', package], timeout=10).stdout.strip()
    if not before:
        raise RuntimeError('Acceptance process is missing')
    dimensions = capture_screen(log, adb, screenshots / 'before.png') if screenshots else None
    resume = mode in ('resume', 'keyboard-resume')
    keyboard = mode in ('keyboard', 'keyboard-resume')
    if resume:
        run(log, adb + ['shell', 'input', 'keyevent', '3'], timeout=10)
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            state = run(log, adb + ['shell', 'dumpsys', 'activity', 'activities'], timeout=10).stdout
            if activity_is_background(state, package):
                break
            time.sleep(0.2)
        else:
            raise RuntimeError('Acceptance Activity did not enter background')
        time.sleep(2)
        run(log, adb + ['shell', 'am', 'start', '-W', '-n', package + '/android.app.NativeActivity'], timeout=30)
        after = run(log, adb + ['shell', 'pidof', package], timeout=10).stdout.strip()
        if before != after:
            raise RuntimeError('Acceptance process was recreated during resume')
        wait_marker(log, adb, token + ' lifecycle resumed')
        if screenshots:
            capture_screen(log, adb, screenshots / 'resumed.png')
    if mode == 'touch':
        if dimensions is None:
            raise RuntimeError('Touch verification needs screenshot dimensions')
        width, height = dimensions
        run(log, adb + ['shell', 'input', 'touchscreen', 'swipe', str(width // 2), str(height // 2),
                        str(width * 6 // 10), str(height // 2), '1500'], timeout=10)
        wait_marker(log, adb, token + ' touch released')
        if rust:
            wait_marker(log, adb, token + ' rust touch released')
        if lua:
            wait_marker(log, adb, token + ' lua touch released')
        capture_screen(log, adb, screenshots / 'released.png')
    if keyboard:
        for keycode, name in (('29', 'a'), ('21', 'left')):
            run(log, adb + ['shell', 'input', 'keyboard', 'keyevent', '--longpress', keycode], timeout=10)
            wait_marker(log, adb, token + ' keyboard ' + name + ' released')
            if rust:
                wait_marker(log, adb, token + ' rust keyboard ' + name + ' released')
            if lua:
                wait_marker(log, adb, token + ' lua keyboard ' + name + ' released')
        wait_marker(log, adb, token + ' keyboard passed')
        if rust:
            wait_marker(log, adb, token + ' rust keyboard passed')
        if lua:
            wait_marker(log, adb, token + ' lua keyboard passed')
            wait_marker(log, adb, token + ' lua keyboard catch-up passed')
        if screenshots:
            capture_screen(log, adb, screenshots / 'keyboard.png')
    run(log, adb + ['shell', 'input', 'keyevent', '4'], timeout=10)
    return {'initial_pid': before, 'resumed_pid': after if resume else None,
            'background_foreground': resume,
            'keyboard_keycodes': [29, 21] if keyboard else None,
            'screen_dimensions': dimensions,
            'touch_gesture': {'start': [width // 2, height // 2],
                              'end': [width * 6 // 10, height // 2], 'duration_ms': 1500}
                             if mode == 'touch' else None}


def prepare_foreground(log, adb):
    # Foreground tests need an awake, unlocked emulator. This does not toggle
    # power or interfere with the controlled Home transition inside a case.
    run(log, adb + ['shell', 'input', 'keyevent', '224'], timeout=10)
    run(log, adb + ['shell', 'wm', 'dismiss-keyguard'], timeout=10)


def settle_installation(log, adb):
    # Package-added resource updates must finish before this cold-start baseline.
    result = run(log, adb + ['shell', 'am', 'wait-for-broadcast-idle'], timeout=30)
    if 'All broadcast queues are idle!' not in result.stdout:
        raise RuntimeError('Android package broadcasts did not reach idle')


def failure_evidence(log, adb, work, package=None):
    for command in (['shell', 'dumpsys', 'power'],
                    ['shell', 'dumpsys', 'window', 'windows'],
                    ['shell', 'dumpsys', 'activity', 'activities'],
                    ['logcat', '-d', '-s', 'raylib:D', 'threaded_app:V', 'MalvadoAcceptance:I', 'AndroidRuntime:E', '*:S'],
                    ['logcat', '-b', 'events', '-d', '-s', 'wm_relaunch_activity:I', 'wm_relaunch_resume_activity:I', 'wm_destroy_activity:I', '*:S']):
        try:
            run(log, adb + command, timeout=10, check=False)
        except (OSError, subprocess.SubprocessError) as error:
            log.write(f"\nCannot collect Android state: {error}\n")
    if package:
        try:
            pid = run(log, adb + ['shell', 'pidof', package], timeout=10, check=False).stdout.strip()
            if pid.isascii() and pid.isdigit():
                run(log, adb + ['shell', 'debuggerd', '-b', pid], timeout=15, check=False)
        except (OSError, subprocess.SubprocessError) as error:
            log.write(f"\nCannot collect Android native stack: {error}\n")
    try:
        capture_screen(log, adb, work / 'failure.png')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        log.write(f"\nCannot capture Android failure screen: {error}\n")
    log.flush()


def android_manifest(package):
    return f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}" android:versionCode="1" android:versionName="test">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<uses-feature android:glEsVersion="0x00020000" android:required="true"/>
<application android:hasCode="false" android:debuggable="true" android:label="Malvado acceptance">
<activity android:name="android.app.NativeActivity" android:exported="true" android:screenOrientation="landscape" android:configChanges="orientation|keyboardHidden|screenSize|screenLayout|smallestScreenSize">
<meta-data android:name="android.app.lib_name" android:value="game"/>
<intent-filter><action android:name="android.intent.action.MAIN"/><category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--language', choices=('c', 'lua', 'rust'), default='c')
    parser.add_argument('--entry', choices=('explicit', 'declarative'), default='explicit')
    parser.add_argument('--failure', choices=('none', 'process', 'missing-image'), default='none')
    parser.add_argument('--lifecycle', choices=('back', 'resume', 'touch', 'keyboard', 'keyboard-resume'))
    parser.add_argument('--audio', action='store_true')
    parser.add_argument('--config', choices=('Debug', 'Release'), default='Debug')
    parser.add_argument('--installed-sdk', action='store_true')
    parser.add_argument('--work', type=Path)
    args = parser.parse_args()
    if args.language == 'rust' and (not args.installed_sdk or (args.audio and args.lifecycle not in (None, 'resume')) or args.lifecycle not in (None, 'back', 'resume', 'touch', 'keyboard', 'keyboard-resume') or args.entry != 'explicit'):
        parser.error('Rust APK verification requires --installed-sdk and currently supports startup/pixels/cleanup, Back, resume, contact, keyboard, expected-error cleanup and normal/resumed audio')
    if args.audio and args.failure != 'none':
        parser.error('--audio cannot be combined with expected failures')
    if args.lifecycle and args.failure != 'none':
        parser.error('--lifecycle cannot be combined with expected failures')
    if args.language == 'lua' and args.installed_sdk:
        parser.error('Lua APK verification currently uses the source runtime')
    if args.language == 'c' and args.failure != 'none':
        parser.error('--failure requires --language lua')
    if args.language == 'c' and args.entry != 'explicit':
        parser.error('--entry requires --language lua')
    default_work = ROOT / ('build-android-lua-window-test' if args.language == 'lua'
                           else 'build-android-window-test')
    work = (args.work or default_work).resolve()
    work.mkdir(parents=True, exist_ok=True)
    sdk = args.sdk.resolve()
    (work / 'results.json').unlink(missing_ok=True)
    for screenshot in ('before.png', 'resumed.png', 'released.png', 'keyboard.png', 'failure.png'):
        (work / screenshot).unlink(missing_ok=True)
    token = uuid.uuid4().hex
    package = 'dev.malvado.acceptance.p' + token
    with (work / 'window.log').open('w') as log:
        ndk, adb, abi, facts = preflight(sdk, args.serial, log)
        tools = packaging_tools(sdk, log, facts)
        build = work / 'native'
        fixture = ROOT / ('tests/android-lua' if args.language == 'lua' else 'tests/android-window')
        lifecycle = {None: 0, 'back': 1, 'resume': 2, 'touch': 3, 'keyboard': 4, 'keyboard-resume': 5}[args.lifecycle]
        failure = {'none': 0, 'process': 1, 'missing-image': 2}[args.failure]
        run(log, ['cmake', '-S', fixture, '-B', build,
                  f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                  f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                  f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}',
                  f'-DACCEPTANCE_FAILURE={failure}', f'-DACCEPTANCE_LIFECYCLE={lifecycle}', f'-DACCEPTANCE_AUDIO={int(args.audio)}', f'-DMALVADO_TEST_ASSETS={"ON" if args.language == "rust" else "OFF"}'])
        run(log, ['cmake', '--build', build, '--target', 'game', '--parallel', '8'])
        if args.installed_sdk:
            staged, installed = work / 'staged sdk', work / 'relocated sdk'
            for path in (staged, installed):
                if path.exists():
                    shutil.rmtree(path)
            run(log, ['cmake', '--install', build / 'engine', '--prefix', staged])
            staged.rename(installed)
            external = work / 'external C project'
            external.mkdir(exist_ok=True)
            for name in ('CMakeLists.txt', 'main.c', 'lifecycle.h', 'keyboard.h', 'audio.h'):
                shutil.copyfile(ROOT / 'tests/android-window' / name, external / name)
            build = external / 'build'
            run(log, ['cmake', '-S', external, '-B', build,
                      f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                      f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                      f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}',
                      f'-DMALVADO_TEST_SDK={installed}', f'-DACCEPTANCE_LIFECYCLE={lifecycle}', f'-DACCEPTANCE_AUDIO={int(args.audio)}'])
            run(log, ['cmake', '--build', build, '--target', 'game', '--parallel', '2'])
        if args.language == 'rust':
            from android_rust_apk import build as build_rust
            build = build_rust(log, work, installed, ndk, adb, args.serial, abi, args.config, token, lifecycle, failure, args.audio)
        manifest = work / 'AndroidManifest.xml'
        manifest.write_text(android_manifest(package))
        unsigned, aligned, apk = [work / name for name in ('unsigned.apk', 'aligned.apk', 'game.apk')]
        run(log, [tools / 'aapt', 'package', '-f', '-M', manifest, '-I', sdk / 'platforms/android-34/android.jar', '-F', unsigned])
        with zipfile.ZipFile(unsigned, 'a') as archive:
            archive.write(build / 'libgame.so', f'lib/{abi}/libgame.so')
            if args.language == 'rust':
                add_rust_assets(archive, args.failure)
            if args.language == 'lua':
                add_lua_assets(archive, args.entry, args.failure, keyboard=args.lifecycle in ('keyboard', 'keyboard-resume'))
            if args.audio:
                add_audio_assets(archive, lua=args.language == 'lua')
        run(log, [tools / 'zipalign', '-f', '4', unsigned, aligned])
        key = work / 'test-only.p12'
        if not key.exists():
            run(log, ['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android',
                      '-alias', 'test', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado test only'])
        run(log, [tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
        run(log, [tools / 'apksigner', 'verify', apk])
        run(log, [tools / 'aapt', 'dump', 'xmltree', apk, 'AndroidManifest.xml'])
        expectation = ('lua assets and shutdown passed' if args.language == 'lua'
                       else 'window pixels and shutdown passed')
        if args.language == 'rust' and args.failure != 'none':
            expectation = 'Rust failure cleanup passed'
        completed = False
        try:
            prepare_foreground(log, adb)
            # Install the whole APK before startup. Incremental APK loading is
            # a separate transport capability, outside this runtime matrix.
            run(log, adb + ['install', '--no-incremental', str(apk)], timeout=90)
            settle_installation(log, adb)
            facts['package_broadcasts_idle'] = True
            run(log, adb + ['shell', 'am', 'start', '-W', '-n', package + '/android.app.NativeActivity'], timeout=45)
            lifecycle_evidence = None
            if args.lifecycle:
                lifecycle_evidence = lifecycle_actions(log, adb, package, token, args.lifecycle, work, lua=args.language == 'lua', rust=args.language == 'rust')
            wait_marker(log, adb, token + ' ' + expectation)
            if args.language == 'rust':
                wait_marker(log, adb, token + ' Rust scheduler and cleanup passed')
                if args.failure == 'none':
                    wait_marker(log, adb, token + ' Rust APK assets passed')
                else:
                    diagnostic = 'Rust missing-image error observed' if args.failure == 'missing-image' else 'Rust process error observed'
                    wait_marker(log, adb, token + ' ' + diagnostic)
                    wait_marker(log, adb, token + ' Rust failure resources closed')
            if args.audio:
                wait_marker(log, adb, token + ' audio API passed')
                if args.language == 'rust':
                    wait_marker(log, adb, token + ' Rust music API passed')
            facts.update(audio=args.audio, lifecycle_evidence=lifecycle_evidence, lifecycle=args.lifecycle, language=args.language, entry=args.entry, failure=args.failure, configuration=args.config, installed_sdk=args.installed_sdk, acceptance=expectation, package=package)
            completed = True
        finally:
            if not completed:
                failure_evidence(log, adb, work, package)
                if args.language == 'lua':
                    run(log, adb + ['shell', 'run-as', package, 'cat', 'files/stderr.txt'], timeout=15, check=False)
                run(log, adb + ['logcat', '-b', 'crash', '-d', '-t', '100'], timeout=15, check=False)
            run(log, adb + ['uninstall', package], timeout=30, check=False)
            remaining = run(log, adb + ['shell', 'pm', 'list', 'packages', package], timeout=15).stdout
            if 'package:' + package in remaining:
                raise RuntimeError('Acceptance APK was not removed')
        (work / 'results.json').write_text(json.dumps(facts, indent=2) + '\n')
    print(f'PASS: {args.language} {args.entry} {args.failure}: {expectation}')


if __name__ == '__main__':
    main()
