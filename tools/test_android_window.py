#!/usr/bin/env python3
"""Build and run a bounded C NativeActivity APK with the existing Android backend."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from pathlib import Path
import json
import os
import subprocess
import shutil
import sys
import time
import uuid
import zipfile
sys.dont_write_bytecode = True
from test_android import ROOT, preflight, run


def add_lua_assets(archive, entry, failure):
    if entry not in ('explicit', 'declarative') or failure not in ('none', 'process', 'missing-image'):
        raise ValueError('Unsupported Lua APK fixture mode')
    fixture = ROOT / 'tests/android-lua'
    archive.write(fixture / (entry + '.lua'), 'assets/main.lua')
    archive.write(fixture / 'behavior.lua', 'assets/behavior.lua')
    archive.write(ROOT / 'tests/fixtures/atlas.json', 'assets/atlas.json')
    archive.writestr('assets/message.bin', bytes([0, 13, 10, 255]))
    if failure != 'missing-image':
        archive.write(ROOT / 'tests/fixtures/red.png', 'assets/red.png')


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


def lifecycle_actions(log, adb, package, token, mode):
    wait_marker(log, adb, token + ' lifecycle ready')
    before = run(log, adb + ['shell', 'pidof', package], timeout=10).stdout.strip()
    if not before:
        raise RuntimeError('Acceptance process is missing')
    if mode == 'resume':
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
    run(log, adb + ['shell', 'input', 'keyevent', '4'], timeout=10)
    return {'initial_pid': before, 'resumed_pid': after if mode == 'resume' else before,
            'background_foreground': mode == 'resume'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--language', choices=('c', 'lua'), default='c')
    parser.add_argument('--entry', choices=('explicit', 'declarative'), default='explicit')
    parser.add_argument('--failure', choices=('none', 'process', 'missing-image'), default='none')
    parser.add_argument('--lifecycle', choices=('back', 'resume'))
    parser.add_argument('--config', choices=('Debug', 'Release'), default='Debug')
    parser.add_argument('--installed-sdk', action='store_true')
    parser.add_argument('--work', type=Path)
    args = parser.parse_args()
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
    token = uuid.uuid4().hex
    package = 'dev.malvado.acceptance.p' + token
    with (work / 'window.log').open('w') as log:
        ndk, adb, abi, facts = preflight(sdk, args.serial, log)
        tools = sdk / 'build-tools/34.0.0'
        for tool in ('aapt', 'zipalign', 'apksigner'):
            if not (tools / tool).is_file():
                raise RuntimeError('Install Android build-tools;34.0.0')
        build = work / 'native'
        fixture = ROOT / ('tests/android-lua' if args.language == 'lua' else 'tests/android-window')
        lifecycle = {None: 0, 'back': 1, 'resume': 2}[args.lifecycle]
        failure = {'none': 0, 'process': 1, 'missing-image': 2}[args.failure]
        run(log, ['cmake', '-S', fixture, '-B', build,
                  f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                  f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                  f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}',
                  f'-DACCEPTANCE_FAILURE={failure}', f'-DACCEPTANCE_LIFECYCLE={lifecycle}'])
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
            for name in ('CMakeLists.txt', 'main.c', 'lifecycle.h'):
                shutil.copyfile(ROOT / 'tests/android-window' / name, external / name)
            build = external / 'build'
            run(log, ['cmake', '-S', external, '-B', build,
                      f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                      f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                      f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}',
                      f'-DMALVADO_TEST_SDK={installed}', f'-DACCEPTANCE_LIFECYCLE={lifecycle}'])
            run(log, ['cmake', '--build', build, '--target', 'game', '--parallel', '2'])
        manifest = work / 'AndroidManifest.xml'
        manifest.write_text(f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}" android:versionCode="1" android:versionName="test">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<uses-feature android:glEsVersion="0x00020000" android:required="true"/>
<application android:hasCode="false" android:debuggable="true" android:label="Malvado acceptance">
<activity android:name="android.app.NativeActivity" android:exported="true" android:screenOrientation="landscape" android:configChanges="orientation|keyboardHidden|screenSize">
<meta-data android:name="android.app.lib_name" android:value="game"/>
<intent-filter><action android:name="android.intent.action.MAIN"/><category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>''')
        unsigned, aligned, apk = [work / name for name in ('unsigned.apk', 'aligned.apk', 'game.apk')]
        run(log, [tools / 'aapt', 'package', '-f', '-M', manifest, '-I', sdk / 'platforms/android-34/android.jar', '-F', unsigned])
        with zipfile.ZipFile(unsigned, 'a') as archive:
            archive.write(build / 'libgame.so', f'lib/{abi}/libgame.so')
            if args.language == 'lua':
                add_lua_assets(archive, args.entry, args.failure)
        run(log, [tools / 'zipalign', '-f', '4', unsigned, aligned])
        key = work / 'test-only.p12'
        if not key.exists():
            run(log, ['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android',
                      '-alias', 'test', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado test only'])
        run(log, [tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
        run(log, [tools / 'apksigner', 'verify', apk])
        expectation = ('lua assets and shutdown passed' if args.language == 'lua'
                       else 'window pixels and shutdown passed')
        completed = False
        try:
            run(log, adb + ['install', str(apk)], timeout=90)
            run(log, adb + ['shell', 'am', 'start', '-W', '-n', package + '/android.app.NativeActivity'], timeout=45)
            lifecycle_evidence = None
            if args.lifecycle:
                lifecycle_evidence = lifecycle_actions(log, adb, package, token, args.lifecycle)
            wait_marker(log, adb, token + ' ' + expectation)
            facts.update(lifecycle_evidence=lifecycle_evidence, lifecycle=args.lifecycle, language=args.language, entry=args.entry, failure=args.failure, configuration=args.config, installed_sdk=args.installed_sdk, acceptance=expectation, package=package)
            completed = True
        finally:
            if not completed:
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
