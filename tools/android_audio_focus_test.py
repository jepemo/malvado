"""Competing Android audio focus for the production C APK verifier."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import time
import uuid
import zipfile
from android_export import java_dex


def companion(checked, sdk, work):
    package = 'dev.malvado.focus.p' + uuid.uuid4().hex
    android = sdk / 'platforms/android-34/android.jar'
    tools = sdk / 'build-tools/34.0.0'
    source = Path(__file__).resolve().parents[1] / 'tests/android-window/FocusReceiver.java'
    dex = java_dex(checked, tools, android, work, source)
    manifest = work / 'AndroidManifest.xml'
    manifest.write_text(f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<application><receiver android:name="dev.malvado.test.FocusReceiver" android:exported="true"/></application>
</manifest>''')
    apk = work / 'focus.apk'
    checked([tools/'aapt', 'package', '-M', manifest, '-I', android, '-F', apk])
    with zipfile.ZipFile(apk, 'a') as archive:
        archive.write(dex, 'classes.dex')
    key = work / 'focus-test.p12'
    checked(['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android',
             '-alias', 'focus', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado focus test'])
    checked([tools/'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', apk])
    return package, apk


def focus_owned(output, package):
    _, header, stack = output.partition(b'Audio Focus stack entries (last is top of stack):\n')
    if not header:
        raise RuntimeError('Android audio focus stack was missing from dumpsys')
    return package.encode() in stack.split(b'\n\n', 1)[0]


def exercise(checked, adb, package, pid, competitor, work, activity):
    from test_android_window import activity_is_background

    def wait_for(command, matches, failure):
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            output = checked(adb + command).stdout
            if matches(output):
                return output
            time.sleep(.2)
        raise RuntimeError(f'{failure}: {output.decode(errors="replace")}')

    def marker(text):
        return wait_for(['logcat', '-d', '--pid='+pid, '-s', 'MalvadoGame:I', '*:S'],
                        lambda output: text.encode() in output, text)

    def request(gain):
        result = checked(adb + ['shell', 'am', 'broadcast', '-n', competitor+'/dev.malvado.test.FocusReceiver',
                                '--ei', 'gain', str(gain)]).stdout
        if b'Broadcast completed: result=1' not in result:
            raise RuntimeError(result.decode(errors='replace'))

    marker('focus initial gain')
    request(2)  # AUDIOFOCUS_GAIN_TRANSIENT
    marker('focus transient mute')
    request(0)
    marker('focus transient gain')
    request(1)  # AUDIOFOCUS_GAIN
    marker('focus permanent mute')
    request(0)
    audio = wait_for(['shell', 'dumpsys', 'audio'], lambda output: not focus_owned(output, package),
                     'Game reacquired focus after a permanent loss')
    (work/'focus-permanent-loss.log').write_bytes(audio)
    checked(adb + ['shell', 'input', 'keyevent', '3'])
    wait_for(['shell', 'dumpsys', 'activity', 'activities'],
             lambda output: activity_is_background(output.decode(errors='replace'), package),
             'Game did not enter background')
    audio = wait_for(['shell', 'dumpsys', 'audio'], lambda output: not focus_owned(output, package),
                     'Game retained audio focus after Home')
    (work/'focus-background.log').write_bytes(audio)
    checked(adb + ['shell', 'am', 'start', '-W', '-n', package+'/'+activity])
    assert checked(adb + ['shell', 'pidof', package]).stdout.decode().strip() == pid
    audio = wait_for(['shell', 'dumpsys', 'audio'], lambda output: focus_owned(output, package),
                     'Game did not regain audio focus')
    (work/'focus-resumed.log').write_bytes(audio)
    checked(adb + ['shell', 'input', 'text', 'r'])
    marker('focus returned after Home')
    (work/'focus-resumed.png').write_bytes(checked(adb + ['exec-out', 'screencap', '-p']).stdout)
    checked(adb + ['shell', 'input', 'keyevent', '4'])
