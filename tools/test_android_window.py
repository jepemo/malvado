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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--config', choices=('Debug', 'Release'), default='Debug')
    parser.add_argument('--installed-sdk', action='store_true')
    parser.add_argument('--work', type=Path, default=ROOT / 'build-android-window-test')
    args = parser.parse_args()
    work = args.work.resolve()
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
        run(log, ['cmake', '-S', ROOT / 'tests/android-window', '-B', build,
                  f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                  f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                  f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}'])
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
            for name in ('CMakeLists.txt', 'main.c'):
                shutil.copyfile(ROOT / 'tests/android-window' / name, external / name)
            build = external / 'build'
            run(log, ['cmake', '-S', external, '-B', build,
                      f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake',
                      f'-DANDROID_ABI={abi}', '-DANDROID_PLATFORM=android-28',
                      f'-DCMAKE_BUILD_TYPE={args.config}', f'-DACCEPTANCE_TOKEN={token}',
                      f'-DMALVADO_TEST_SDK={installed}'])
            run(log, ['cmake', '--build', build, '--target', 'game', '--parallel', '2'])
        manifest = work / 'AndroidManifest.xml'
        manifest.write_text(f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}" android:versionCode="1" android:versionName="test">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<uses-feature android:glEsVersion="0x00020000" android:required="true"/>
<application android:hasCode="false" android:label="Malvado acceptance">
<activity android:name="android.app.NativeActivity" android:exported="true" android:screenOrientation="landscape" android:configChanges="orientation|keyboardHidden|screenSize">
<meta-data android:name="android.app.lib_name" android:value="game"/>
<intent-filter><action android:name="android.intent.action.MAIN"/><category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>''')
        unsigned, aligned, apk = [work / name for name in ('unsigned.apk', 'aligned.apk', 'game.apk')]
        run(log, [tools / 'aapt', 'package', '-f', '-M', manifest, '-I', sdk / 'platforms/android-34/android.jar', '-F', unsigned])
        with zipfile.ZipFile(unsigned, 'a') as archive:
            archive.write(build / 'libgame.so', f'lib/{abi}/libgame.so')
        run(log, [tools / 'zipalign', '-f', '4', unsigned, aligned])
        key = work / 'test-only.p12'
        if not key.exists():
            run(log, ['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android',
                      '-alias', 'test', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado test only'])
        run(log, [tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
        run(log, [tools / 'apksigner', 'verify', apk])
        completed = False
        try:
            run(log, adb + ['install', str(apk)], timeout=90)
            run(log, adb + ['shell', 'am', 'start', '-W', '-n', package + '/android.app.NativeActivity'], timeout=45)
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                output = run(log, adb + ['logcat', '-d', '-s', 'MalvadoAcceptance:I', '*:S'], timeout=10).stdout
                if token + ' window pixels and shutdown passed' in output:
                    break
                time.sleep(1)
            else:
                raise RuntimeError('Android window acceptance did not complete within 30 seconds')
            facts.update(configuration=args.config, installed_sdk=args.installed_sdk, acceptance='window pixels and shutdown passed', package=package)
            completed = True
        finally:
            if not completed:
                run(log, adb + ['logcat', '-b', 'crash', '-d', '-t', '100'], timeout=15, check=False)
            run(log, adb + ['uninstall', package], timeout=30, check=False)
            remaining = run(log, adb + ['shell', 'pm', 'list', 'packages', package], timeout=15).stdout
            if 'package:' + package in remaining:
                raise RuntimeError('Acceptance APK was not removed')
        (work / 'results.json').write_text(json.dumps(facts, indent=2) + '\n')
    print('PASS: NativeActivity startup, GLES pixels and shutdown')


if __name__ == '__main__':
    main()
