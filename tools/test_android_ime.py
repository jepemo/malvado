#!/usr/bin/env python3
"""Run real committed-text checks for exported games using a dedicated test IME."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import time
import uuid
import zipfile
from android_export import java_dex


def wait_for_ime(command, component):
    # Package broadcasts can finish before InputMethodManager refreshes its list.
    deadline = time.monotonic() + 15
    while time.monotonic() < deadline:
        available = command(['ime', 'list', '-a', '-s']).splitlines()
        if component in available:
            return
        time.sleep(.2)
    raise RuntimeError(f'Android did not register the test IME: {component}')


def keyboard_sequence(command, adb, work, mode, pid):
    def wait(shown):
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            state = command(adb + ['shell', 'dumpsys', 'input_method']).stdout
            focused = next((line for line in state.splitlines() if b'mCurFocusedWindow=' in line), b'')
            if (f'mInputShown={str(shown).lower()}'.encode() in state and
                    f'mPid={pid} '.encode() in focused):
                name = 'open' if shown else 'closed'
                (work/f'{mode}-ime-{name}.log').write_bytes(state)
                screenshot = command(adb + ['exec-out', 'screencap', '-p']).stdout
                if not screenshot.startswith(b'\x89PNG\r\n\x1a\n'):
                    raise RuntimeError('Android IME screenshot is not a PNG')
                (work/f'{mode}-ime-{name}.png').write_bytes(screenshot)
                return
            time.sleep(.1)
        raise RuntimeError(f'Android did not report keyboard visible={shown}')

    command(adb + ['shell', 'input', 'text', 'k'])
    wait(True)
    wait(False)


def create_ime(sdk, work):
    def checked(arguments):
        subprocess.run([str(value) for value in arguments], check=True, timeout=120)

    package = 'dev.malvado.inputtest.p' + uuid.uuid4().hex
    android = sdk/'platforms/android-34/android.jar'
    tools = sdk/'build-tools/34.0.0'
    source = Path(__file__).resolve().parents[1]/'tests/android-window/TestInputMethod.java'
    dex = java_dex(checked, tools, android, work, source)
    manifest = work/'AndroidManifest.xml'
    manifest.write_text(f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<application android:label="Malvado test IME">
<service android:name="dev.malvado.test.TestInputMethod" android:permission="android.permission.BIND_INPUT_METHOD" android:exported="true">
<intent-filter><action android:name="android.view.InputMethod"/></intent-filter>
<meta-data android:name="android.view.im" android:resource="@xml/input_method"/>
</service></application></manifest>''')
    xml = work/'res/xml'
    xml.mkdir(parents=True)
    (xml/'input_method.xml').write_text('<input-method xmlns:android="http://schemas.android.com/apk/res/android"/>')
    apk = work/'ime.apk'
    checked([tools/'aapt', 'package', '-M', manifest, '-S', work/'res', '-I', android, '-F', apk])
    with zipfile.ZipFile(apk, 'a') as archive:
        archive.write(dex, 'classes.dex')
    aligned = work/'ime-aligned.apk'
    checked([tools/'zipalign', '-f', '4', apk, aligned])
    key = work/'ime.p12'
    checked(['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-alias', 'ime',
             '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado test IME'])
    checked([tools/'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', aligned])
    return package, aligned


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cli', 'android-sdk', 'lua-sdk', 'native-sdk', 'work'):
        parser.add_argument('--'+name, required=True, type=Path)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--release', action='store_true')
    parser.add_argument('--format', choices=('apk', 'aab'), default='apk')
    parser.add_argument('--bundle-tools', type=Path)
    parser.add_argument('--keystore', type=Path)
    parser.add_argument('--key-alias')
    parser.add_argument('--store-password-env')
    args = parser.parse_args()
    if not args.serial.startswith('emulator-') or not args.serial[9:].isdigit():
        parser.error('Select the dedicated test emulator by its serial')
    if args.release and not all((args.keystore, args.key_alias, args.store_password_env)):
        parser.error('Release checks require disposable signing credentials')
    if args.format == 'aab' and (not args.release or not args.bundle_tools):
        parser.error('Bundle verification requires --release and --bundle-tools')
    args.work.mkdir(parents=True, exist_ok=True)
    adb = [args.android_sdk.resolve()/'platform-tools/adb', '-s', args.serial]

    def command(arguments, *, check=True):
        result = subprocess.run([str(value) for value in adb + ['shell'] + arguments],
                                capture_output=True, text=True, timeout=30)
        if check and result.returncode:
            raise RuntimeError(f'adb shell {shlex.join(arguments)} failed: {result.stderr.strip()}')
        return result.stdout.strip()

    default = command(['settings', 'get', 'secure', 'default_input_method'])
    hard_keyboard = command(['settings', 'get', 'secure', 'show_ime_with_hard_keyboard'])
    if not default or default == 'null':
        raise RuntimeError('The test emulator needs a default input method to restore')
    with tempfile.TemporaryDirectory(prefix='malvado-test-ime-', dir=args.work.resolve()) as folder:
        package, apk = create_ime(args.android_sdk.resolve(), Path(folder))
        component = package+'/dev.malvado.test.TestInputMethod'
        subprocess.run([*map(str, adb), 'install', '--no-incremental', str(apk)], check=True, timeout=60)
        try:
            command(['am', 'wait-for-broadcast-idle'])
            wait_for_ime(command, component)
            command(['ime', 'enable', component])
            command(['ime', 'set', component])
            assert command(['settings', 'get', 'secure', 'default_input_method']) == component
            command(['settings', 'put', 'secure', 'show_ime_with_hard_keyboard', '1'])
            tools = Path(__file__).resolve().parent
            for language, program, sdk in (
                ('c', 'test_android_c_export.py', args.native_sdk),
                ('lua', 'test_android_export.py', args.lua_sdk),
                ('rust', 'test_android_rust_export.py', args.native_sdk),
            ):
                flags = ['--format', args.format]
                if args.format == 'aab':
                    flags += ['--bundle-tools', args.bundle_tools]
                if args.release:
                    flags.append('--release')
                if args.release and language != 'lua':
                    flags += ['--keystore', args.keystore, '--key-alias', args.key_alias,
                              '--store-password-env', args.store_password_env]
                call = [sys.executable, tools/program, '--cli', args.cli, '--runtime-sdk', sdk,
                        '--android-sdk', args.android_sdk, '--serial', args.serial,
                        '--work', args.work/language, '--ime', *flags]
                print(shlex.join(map(str, call)), flush=True)
                subprocess.run(list(map(str, call)), check=True, timeout=600)
        finally:
            command(['ime', 'set', default], check=False)
            command(['ime', 'disable', component], check=False)
            if not hard_keyboard or hard_keyboard == 'null':
                command(['settings', 'delete', 'secure', 'show_ime_with_hard_keyboard'], check=False)
            else:
                command(['settings', 'put', 'secure', 'show_ime_with_hard_keyboard', hard_keyboard], check=False)
            subprocess.run([*map(str, adb), 'uninstall', package], check=True, timeout=60)
            assert command(['settings', 'get', 'secure', 'default_input_method']) == default
            assert command(['settings', 'get', 'secure', 'show_ime_with_hard_keyboard']) == hard_keyboard
            assert ('package:'+package) not in command(['pm', 'list', 'packages', package])


if __name__ == '__main__':
    main()
