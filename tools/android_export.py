#!/usr/bin/env python3
"""Export a Lua project as an Android arm64 debug APK using a target SDK runtime."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile
from xml.sax.saxutils import quoteattr


def project_files(project):
    result = []
    def visit(folder, prefix=''):
        for path in sorted(folder.iterdir()):
            if path.name.startswith('.') or path.name in ('build', 'dist'):
                continue
            name = prefix + path.name
            if path.is_symlink():
                raise ValueError('Project links are not supported: ' + name)
            if any(part in ('', '.', '..') for part in name.split('/')) or any(c in name for c in ('\\', '\r', '\n', '\0')) or len(name.encode()) >= 1024:
                raise ValueError('Invalid packaged path: ' + repr(name))
            if name in ('malvado-files.txt', 'licenses'):
                raise ValueError('Reserved Android asset name: ' + name)
            if path.is_dir():
                visit(path, name + '/')
            elif path.is_file():
                result.append((path, name))
            else:
                raise ValueError('Project special files are not supported: ' + name)
    visit(project)
    if not any(name == 'main.lua' for _, name in result):
        raise ValueError('Android Lua exports require main.lua')
    return result


def validate_runtime(runtime, version):
    info = dict(line.split('=', 1) for line in (runtime / 'share/malvado/malvado-sdk-info.txt').read_text().splitlines())
    if info.get('system') != 'android' or info.get('version') != version or info.get('architecture') not in ('aarch64', 'arm64', 'arm64-v8a'):
        raise ValueError('Android runtime SDK version/system mismatch')
    library = runtime / 'libexec/malvado/android/arm64-v8a/libgame.so'
    with library.open('rb') as stream:
        header = stream.read(20)
    if len(header) != 20 or header[:7] != b'\x7fELF\x02\x01\x01' or header[16:18] != b'\x03\x00' or header[18:20] != b'\xb7\x00':
        raise ValueError('Android runtime must be an arm64 ELF library')
    return library


def run(arguments):
    subprocess.run([str(arg) for arg in arguments], check=True, timeout=120)


def manifest_xml(package, version_code=1, version_name='debug', title='Malvado game', orientation='landscape', min_api=28):
    if not re.fullmatch(r'[a-zA-Z][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9_]*)+', package):
        raise ValueError('Invalid Android application ID')
    if type(version_code) is not int or not 1 <= version_code <= 2100000000:
        raise ValueError('Android version code must be between 1 and 2100000000')
    if type(min_api) is not int or not 28 <= min_api <= 34:
        raise ValueError('Android minimum API must be between 28 and 34')
    if orientation not in ('landscape', 'portrait', 'sensor'):
        raise ValueError('Unsupported Android orientation')
    for value in (version_name, title):
        if not value or len(value) > 128 or any(ord(c) < 32 or ord(c) == 127 or 0xd800 <= ord(c) <= 0xdfff or ord(c) in (0xfffe, 0xffff) for c in value):
            raise ValueError('Android title/version name must contain 1–128 valid XML characters without controls')
    return f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}" android:versionCode="{version_code}" android:versionName={quoteattr(version_name)}>
<uses-sdk android:minSdkVersion="{min_api}" android:targetSdkVersion="34"/>
<uses-feature android:glEsVersion="0x00020000" android:required="true"/>
<application android:hasCode="false" android:debuggable="true" android:label={quoteattr(title)}>
<activity android:name="android.app.NativeActivity" android:exported="true" android:screenOrientation="{orientation}" android:configChanges="orientation|keyboardHidden|screenSize|screenLayout|smallestScreenSize">
<meta-data android:name="android.app.lib_name" android:value="game"/>
<intent-filter><action android:name="android.intent.action.MAIN"/><category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>'''


def export(project, runtime, sdk, output, package, version, **configuration):
    manifest_text = manifest_xml(package, **configuration)
    if output.suffix != '.apk' or output.exists() or output.is_symlink():
        raise ValueError('Output must be a new .apk file')
    files = project_files(project)
    library = validate_runtime(runtime, version)
    notices = runtime / 'share/licenses/malvado'
    for name in ('LICENSE', 'Lua-readme.html', 'raylib-LICENSE', 'cJSON-LICENSE'):
        path = notices / name
        if not path.is_file():
            raise ValueError('Android runtime license is missing: ' + name)
        files.append((path, 'licenses/' + name))
    tools = sdk / 'build-tools/34.0.0'
    platform = sdk / 'platforms/android-34/android.jar'
    for tool in (tools / 'aapt', tools / 'zipalign', tools / 'apksigner', platform):
        if not tool.is_file():
            raise ValueError('Android SDK requires Build Tools 34.0.0 and platform 34')
    def properties(path):
        return dict((key.strip(), value.strip()) for line in path.read_text().splitlines()
                    if '=' in line and not line.lstrip().startswith('#')
                    for key, value in [line.split('=', 1)])
    if properties(tools / 'source.properties').get('Pkg.Revision') != '34.0.0' or properties(sdk / 'platforms/android-34/source.properties').get('AndroidVersion.ApiLevel') != '34':
        raise ValueError('Android SDK version metadata does not match Build Tools 34.0.0 / API 34')
    for tool in ('java', 'keytool'):
        if not shutil.which(tool):
            raise ValueError('Required Android export tool is missing: ' + tool)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.malvado-apk-', dir=output.parent) as folder:
        work = Path(folder)
        manifest = work / 'AndroidManifest.xml'
        manifest.write_text(manifest_text, encoding='utf-8')
        unsigned, aligned, apk = [work / name for name in ('unsigned.apk', 'aligned.apk', 'game.apk')]
        run([tools / 'aapt', 'package', '-f', '-M', manifest, '-I', platform, '-F', unsigned])
        with zipfile.ZipFile(unsigned, 'a') as archive:
            archive.write(library, 'lib/arm64-v8a/libgame.so')
            for path, name in files:
                archive.write(path, 'assets/' + name)
            archive.writestr('assets/malvado-files.txt', ''.join(name + '\n' for _, name in files))
        run([tools / 'zipalign', '-f', '4', unsigned, aligned])
        key = work / 'debug.p12'
        run(['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android', '-alias', 'debug', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado debug only'])
        run([tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
        run([tools / 'apksigner', 'verify', apk])
        os.link(apk, output)  # Atomic exclusive publication; never replace a file.
    print('Exported debug APK: ' + str(output))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('android-arm64',), required=True)
    parser.add_argument('--format', choices=('apk',), required=True)
    parser.add_argument('--runtime-sdk', type=Path, required=True)
    parser.add_argument('--android-sdk', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--id', default='org.malvado.game')
    parser.add_argument('--version-code', type=int, default=1)
    parser.add_argument('--version-name', default='debug')
    parser.add_argument('--title', default='Malvado game')
    parser.add_argument('--orientation', choices=('landscape', 'portrait', 'sensor'), default='landscape')
    parser.add_argument('--min-api', type=int, default=28)
    parser.add_argument('--engine-version', required=True)
    args = parser.parse_args()
    try:
        export(Path.cwd(), args.runtime_sdk.resolve(), args.android_sdk.resolve(), args.output.absolute(), args.id, args.engine_version, version_code=args.version_code, version_name=args.version_name, title=args.title, orientation=args.orientation, min_api=args.min_api)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
