#!/usr/bin/env python3
"""Export a C or Lua project as an Android arm64 APK using a matching target SDK."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile
from xml.sax.saxutils import quoteattr


def project_files(project, language='lua'):
    result = []
    def visit(folder, prefix=''):
        for path in sorted(folder.iterdir()):
            if path.name.startswith('.') or path.name in ('build', 'dist') or (language == 'rust' and path.name == 'target'):
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
                if language == 'lua' or (language == 'c' and path.suffix not in ('.c', '.h') and path.name != 'CMakeLists.txt') or (language == 'rust' and path.suffix not in ('.rs', '.c', '.h') and path.name not in ('Cargo.toml', 'Cargo.lock', 'CMakeLists.txt')):
                    result.append((path, name))
            else:
                raise ValueError('Project special files are not supported: ' + name)
    visit(project)
    if language == 'rust' and not (project / 'Cargo.toml').is_file():
        raise ValueError('Android Rust exports require Cargo.toml with a staticlib target')
    if language == 'c' and not (project / 'CMakeLists.txt').is_file():
        raise ValueError('Android C exports require CMakeLists.txt with a game target')
    if language == 'lua' and not any(name == 'main.lua' for _, name in result):
        raise ValueError('Android Lua exports require main.lua')
    return result


def validate_sdk(runtime, version):
    info = dict(line.split('=', 1) for line in (runtime / 'share/malvado/malvado-sdk-info.txt').read_text().splitlines())
    if info.get('system') != 'android' or info.get('version') != version or info.get('architecture') not in ('aarch64', 'arm64', 'arm64-v8a'):
        raise ValueError('Android runtime SDK version/system mismatch')


def validate_library(library):
    with library.open('rb') as stream:
        header = stream.read(20)
    if len(header) != 20 or header[:7] != b'\x7fELF\x02\x01\x01' or header[16:18] != b'\x03\x00' or header[18:20] != b'\xb7\x00':
        raise ValueError('Android runtime must be an arm64 ELF library')
    return library


def validate_runtime(runtime, version, release=False):
    validate_sdk(runtime, version)
    library = validate_library(runtime / 'libexec/malvado/android/arm64-v8a/libgame.so')
    if release:
        metadata = runtime / 'share/malvado/android-runtime.txt'
        if not metadata.is_file():
            raise ValueError('Release export requires runtime profile metadata; install AndroidRuntime in Release')
        profile = dict(line.split('=', 1) for line in metadata.read_text().splitlines())
        if profile != dict(language='lua', configuration='Release', abi='arm64-v8a'):
            raise ValueError('Release APK export requires an arm64 Lua runtime built in Release')
    return library


def run(arguments):
    subprocess.run([str(arg) for arg in arguments], check=True, timeout=120)


def build_c_game(project, runtime, sdk, work, release, ndk=None):
    ndk = Path(ndk).resolve() if ndk else sdk / 'ndk/27.2.12479018'
    properties = ndk / 'source.properties'
    if not properties.is_file() or not re.search(r'^Pkg.Revision\s*=\s*27\.2\.12479018\s*$', properties.read_text(), re.MULTILINE):
        raise ValueError('C Android export requires NDK r27c (27.2.12479018)')
    if not (runtime / 'lib/cmake/Malvado/MalvadoConfig.cmake').is_file():
        raise ValueError('C export requires a full installed Android graphical SDK')
    if release and not (runtime / 'lib/cmake/Malvado/MalvadoGraphicsTargets-release.cmake').is_file():
        raise ValueError('Release C export requires a full Android SDK installed in Release')
    if not shutil.which('cmake'):
        raise ValueError('C export requires CMake on PATH')
    build = work / 'native'
    run(['cmake', '-S', project, '-B', build,
         '-DCMAKE_TOOLCHAIN_FILE=' + str(ndk / 'build/cmake/android.toolchain.cmake'),
         '-DANDROID_ABI=arm64-v8a', '-DANDROID_PLATFORM=android-28',
         '-DCMAKE_BUILD_TYPE=' + ('Release' if release else 'Debug'),
         '-DMalvado_DIR=' + str(runtime / 'lib/cmake/Malvado'),
         '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=' + str(build / 'output'),
         '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(build / 'output')])
    run(['cmake', '--build', build, '--target', 'game', '--parallel', '8'])
    return validate_native_dependencies(build / 'output/libgame.so', ndk)


def validate_native_dependencies(library, ndk):
    library = validate_library(library)
    inspectors = list((ndk / 'toolchains/llvm/prebuilt').glob('*/bin/llvm-readelf'))
    if len(inspectors) != 1:
        raise ValueError('Pinned NDK LLVM dependency inspector is missing')
    inspected = subprocess.run([str(inspectors[0]), '--dynamic', str(library)], check=True,
                               capture_output=True, text=True, timeout=30)
    needed = set(re.findall(r'\(NEEDED\).*\[([^]]+)\]', inspected.stdout))
    platform_libraries = {'libc.so', 'libm.so', 'libdl.so', 'liblog.so', 'libandroid.so',
                          'libEGL.so', 'libGLESv2.so', 'libOpenSLES.so'}
    if needed - platform_libraries:
        raise ValueError('C export has unsupported shared dependencies: ' + ', '.join(sorted(needed - platform_libraries)))
    return library


def build_rust_game(project, runtime, sdk, work, release, ndk=None, probe_runner=None):
    ndk = Path(ndk).resolve() if ndk else sdk / 'ndk/27.2.12479018'
    runner = probe_runner or os.environ.get('MALVADO_ANDROID_RUNNER')
    if not runner or not Path(runner).is_file() or not os.access(runner, os.X_OK):
        raise ValueError('Rust export requires an executable --probe-runner for target ABI verification')
    if Path(runner).resolve().is_relative_to(project.resolve()):
        raise ValueError('Rust probe runner must be outside the project assets')
    for tool in ('cargo', 'cmake'):
        if not shutil.which(tool):
            raise ValueError('Rust export requires Cargo and CMake on PATH')
    metadata = subprocess.run(['cargo', 'metadata', '--format-version', '1', '--no-deps',
                               '--manifest-path', str(project / 'Cargo.toml')], check=True,
                              capture_output=True, text=True, timeout=120)
    catalog = json.loads(metadata.stdout)
    packages = [package for package in catalog['packages'] if Path(package['manifest_path']).resolve() == (project / 'Cargo.toml').resolve()]
    if len(packages) != 1:
        raise ValueError('Rust export requires one package at the project root')
    targets = [target for target in packages[0]['targets'] if 'staticlib' in target['crate_types']]
    if len(targets) != 1:
        raise ValueError('Rust export requires exactly one staticlib target with android_entry!')
    if not any(dependency.get('name') == 'malvado' and not dependency.get('optional', False) and dependency.get('target') is None for dependency in packages[0].get('dependencies', [])):
        raise ValueError('Rust export requires a direct non-optional malvado SDK dependency for ABI checks')
    linkers = list((ndk / 'toolchains/llvm/prebuilt').glob('*/bin/aarch64-linux-android28-clang'))
    if len(linkers) != 1:
        raise ValueError('Rust export requires NDK r27c arm64 linker')
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(runtime), MALVADO_ANDROID_NDK=str(ndk),
               MALVADO_ANDROID_RUNNER=str(Path(runner).resolve()),
               CARGO_TARGET_DIR=str(work / 'cargo'),
               CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER=str(linkers[0]), RUSTUP_AUTO_INSTALL='0')
    command = ['cargo', 'build', '--manifest-path', str(project / 'Cargo.toml'), '--lib', '--package', packages[0]['name'], '--target', 'aarch64-linux-android']
    if release:
        command.append('--release')
    subprocess.run(command, env=env, check=True, timeout=300)
    archive = work / 'cargo/aarch64-linux-android' / ('release' if release else 'debug') / ('lib'+targets[0]['name'].replace('-', '_')+'.a')
    if not archive.is_file():
        raise ValueError('Cargo did not produce the expected Android static library')
    bridge = work / 'bridge'
    bridge.mkdir()
    # JSON quoted strings also quote ordinary CMake paths without shell parsing.
    for value in (str(runtime), str(archive)):
        if any(c in value for c in (';', '$', '\n', '\r', '"', '\\')):
            raise ValueError('Unsupported character in Rust Android build path')
    (bridge / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(game C)\nfind_package(Malvado CONFIG REQUIRED COMPONENTS graphics)\nmalvado_add_android_rust_game(game '+json.dumps(str(archive))+')\n')
    return build_c_game(bridge, runtime, sdk, work, release, ndk)


GAME_ACTIVITY = "dev.malvado.player.GameActivity"

def manifest_xml(package, version_code=1, version_name='debug', title='Malvado game', orientation='landscape', min_api=28, release=False):
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
<application android:hasCode="true" android:debuggable="{str(not release).lower()}" android:label={quoteattr(title)}>
<activity android:name="{GAME_ACTIVITY}" android:exported="true" android:screenOrientation="{orientation}" android:configChanges="orientation|keyboardHidden|screenSize|screenLayout|smallestScreenSize">
<meta-data android:name="android.app.lib_name" android:value="game"/>
<intent-filter><action android:name="android.intent.action.MAIN"/><category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>'''


def activity_dex(tools, platform, work):
    classes = work / 'classes'
    classes.mkdir()
    source = Path(__file__).resolve().with_name('GameActivity.java')
    if not source.is_file():
        source = Path(__file__).resolve().parents[1] / 'src/android/GameActivity.java'
    run(['javac', '--release', '8', '-classpath', platform, '-d', classes, source])
    run([tools / 'd8', '--lib', platform, '--min-api', '28', '--output', work,
         *sorted(classes.rglob('*.class'))])
    return work / 'classes.dex'


def signing_configuration(project, keystore=None, key_alias=None, store_password_env=None, key_password_env=None):
    if keystore is None:
        if any(value is not None for value in (key_alias, store_password_env, key_password_env)):
            raise ValueError('Signing options require --keystore')
        return None
    keystore = Path(keystore).resolve()
    if not keystore.is_file() or keystore.is_relative_to(project.resolve()):
        raise ValueError('Keystore must be an existing file outside the project')
    if not key_alias or len(key_alias) > 128 or any(ord(c) < 32 for c in key_alias):
        raise ValueError('Signing requires a valid --key-alias')
    if not store_password_env:
        raise ValueError('Signing requires --store-password-env')
    names = [store_password_env] + ([key_password_env] if key_password_env is not None else [])
    for name in names:
        if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', name) or not os.environ.get(name):
            raise ValueError('Signing password environment variable is missing or invalid')
    return keystore, key_alias, store_password_env, key_password_env


def export(project, runtime, sdk, output, package, version, *, release=False, keystore=None, key_alias=None, store_password_env=None, key_password_env=None, ndk=None, probe_runner=None, output_format='apk', bundle_tools=None, **configuration):
    if release:
        configuration.setdefault('version_name', 'release')
    manifest_text = manifest_xml(package, release=release, **configuration)
    signing = signing_configuration(project, keystore, key_alias, store_password_env, key_password_env)
    if output_format not in ('apk', 'aab'):
        raise ValueError('Unsupported Android export format')
    if output.suffix != '.'+output_format or output.exists() or output.is_symlink():
        raise ValueError('Output must be a new .'+output_format+' file')
    if output_format == 'aab':
        if not release:
            raise ValueError('AAB export requires --release')
        if bundle_tools is None:
            raise ValueError('AAB export requires --bundle-tools')
        from android_bundle import validate_tools
        validate_tools(Path(bundle_tools))
    language = 'lua' if (project / 'main.lua').is_file() else ('rust' if (project / 'Cargo.toml').is_file() else 'c')
    files = project_files(project, language)
    if language == 'lua':
        library = validate_runtime(runtime, version, release=release)
    else:
        validate_sdk(runtime, version)
    notices = runtime / 'share/licenses/malvado'
    names = ['LICENSE', 'Lua-readme.html', 'raylib-LICENSE', 'cJSON-LICENSE'] if language == 'lua' else ['LICENSE', 'raylib-LICENSE']
    if language != 'lua' and (notices / 'cJSON-LICENSE').is_file():
        names.append('cJSON-LICENSE')
    for name in names:
        path = notices / name
        if not path.is_file():
            raise ValueError('Android runtime license is missing: ' + name)
        files.append((path, 'licenses/' + name))
    tools = sdk / 'build-tools/34.0.0'
    platform = sdk / 'platforms/android-34/android.jar'
    required_tools = [platform] if output_format == 'aab' else [tools / 'aapt', tools / 'zipalign', platform]
    required_tools.append(tools / 'd8')
    if output_format == 'apk' and (not release or signing):
        required_tools.append(tools / 'apksigner')
    for tool in required_tools:
        if not tool.is_file():
            raise ValueError('Android SDK requires Build Tools 34.0.0 (including D8) and platform 34')
    def properties(path):
        return dict((key.strip(), value.strip()) for line in path.read_text().splitlines()
                    if '=' in line and not line.lstrip().startswith('#')
                    for key, value in [line.split('=', 1)])
    if properties(tools / 'source.properties').get('Pkg.Revision') != '34.0.0' or properties(sdk / 'platforms/android-34/source.properties').get('AndroidVersion.ApiLevel') != '34':
        raise ValueError('Android SDK version metadata does not match Build Tools 34.0.0 / API 34')
    host_tools = ['java', 'javac']
    if not release and signing is None:
        host_tools.append('keytool')
    if output_format == 'aab' and signing:
        host_tools.append('jarsigner')
    for tool in host_tools:
        if not shutil.which(tool):
            raise ValueError('Required Android export tool is missing: ' + tool)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.malvado-apk-', dir=output.parent) as folder:
        work = Path(folder)
        if language == 'rust':
            library = build_rust_game(project, runtime, sdk, work, release, ndk, probe_runner)
        if language == 'c':
            library = build_c_game(project, runtime, sdk, work, release, ndk)
        manifest = work / 'AndroidManifest.xml'
        manifest.write_text(manifest_text, encoding='utf-8')
        dex = activity_dex(tools, platform, work)
        if output_format == 'aab':
            from android_bundle import build as build_bundle
            artifact = build_bundle(run, Path(bundle_tools), sdk, work, manifest, library, files, dex)
            if signing:
                from android_bundle import sign as sign_bundle
                sign_bundle(run, artifact, signing)
            os.link(artifact, output)
            print('Exported ' + ('signed' if signing else 'unsigned') + ' release AAB: ' + str(output))
            return
        unsigned, aligned, apk = [work / name for name in ('unsigned.apk', 'aligned.apk', 'game.apk')]
        run([tools / 'aapt', 'package', '-f', '-M', manifest, '-I', platform, '-F', unsigned])
        with zipfile.ZipFile(unsigned, 'a') as archive:
            archive.write(library, 'lib/arm64-v8a/libgame.so')
            archive.write(dex, 'classes.dex')
            for path, name in files:
                archive.write(path, 'assets/' + name)
            archive.writestr('assets/malvado-files.txt', ''.join(name + '\n' for _, name in files))
        run([tools / 'zipalign', '-f', '4', unsigned, aligned])
        artifact = aligned
        if not release and signing is None:
            key = work / 'debug.p12'
            run(['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android', '-alias', 'debug', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado debug only'])
            run([tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
            artifact = apk
        elif signing:
            key, alias, store_env, key_env = signing
            arguments = [tools / 'apksigner', 'sign', '--ks', key, '--ks-key-alias', alias,
                         '--ks-pass', 'env:' + store_env, '--out', apk]
            if key_env:
                arguments += ['--key-pass', 'env:' + key_env]
            arguments.append(aligned)
            run(arguments)
            artifact = apk
        if artifact == apk:
            run([tools / 'apksigner', 'verify', apk])
        else:
            run([tools / 'zipalign', '-c', '4', aligned])
        os.link(artifact, output)  # Atomic exclusive publication; never replace a file.
    kind = ('signed release' if signing else 'unsigned release') if release else 'debug'
    print('Exported ' + kind + ' APK: ' + str(output))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('android-arm64',), required=True)
    parser.add_argument('--format', choices=('apk', 'aab'), required=True)
    parser.add_argument('--bundle-tools', type=Path)
    parser.add_argument('--probe-runner', type=Path)
    parser.add_argument('--ndk', type=Path)
    parser.add_argument('--runtime-sdk', type=Path, required=True)
    parser.add_argument('--android-sdk', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--id', default='org.malvado.game')
    parser.add_argument('--release', action='store_true')
    parser.add_argument('--keystore', type=Path)
    parser.add_argument('--key-alias')
    parser.add_argument('--store-password-env')
    parser.add_argument('--key-password-env')
    parser.add_argument('--version-code', type=int, default=1)
    parser.add_argument('--version-name')
    parser.add_argument('--title', default='Malvado game')
    parser.add_argument('--orientation', choices=('landscape', 'portrait', 'sensor'), default='landscape')
    parser.add_argument('--min-api', type=int, default=28)
    parser.add_argument('--engine-version', required=True)
    args = parser.parse_args()
    try:
        export(Path.cwd(), args.runtime_sdk.resolve(), args.android_sdk.resolve(), args.output.absolute(), args.id, args.engine_version, output_format=args.format, bundle_tools=args.bundle_tools.resolve() if args.bundle_tools else None, version_code=args.version_code, probe_runner=args.probe_runner, ndk=args.ndk, release=args.release, keystore=args.keystore, key_alias=args.key_alias, store_password_env=args.store_password_env, key_password_env=args.key_password_env, version_name=args.version_name if args.version_name is not None else ('release' if args.release else 'debug'), title=args.title, orientation=args.orientation, min_api=args.min_api)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
