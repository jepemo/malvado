#!/usr/bin/env python3
"""Install pinned AAB tools and build a native base-module app bundle."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import hashlib
import json
from pathlib import Path
import platform
import shutil
import tempfile
import urllib.request
import zipfile

BUNDLETOOL_VERSION = '1.18.3'
BUNDLETOOL_SHA256 = 'a099cfa1543f55593bc2ed16a70a7c67fe54b1747bb7301f37fdfd6d91028e29'
AAPT2_VERSION = '8.2.2-10154469'
AAPT2 = {
    'macos': ('osx', 'e8082544c1f568a2447eab9e0e6b9f80cba7bd4a53892f484bafc0c78f809924'),
    'linux': ('linux', '80c4ce1dbad130ef5600f46c911318d3eff87698425e1aa94c40712d8d8cb2f1'),
}


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def host_name():
    host = {'Darwin': 'macos', 'Linux': 'linux'}.get(platform.system())
    if host is None:
        raise ValueError('AAB authoring requires macOS or Linux')
    return host


def validate_tools(folder, host=None):
    host = host or host_name()
    if host not in AAPT2:
        raise ValueError('Unsupported AAB tool host')
    if digest(folder/'bundletool.jar') != BUNDLETOOL_SHA256 or digest(folder/'aapt2.jar') != AAPT2[host][1]:
        raise ValueError('Pinned AAB tool archive checksum mismatch')
    with zipfile.ZipFile(folder/'aapt2.jar') as archive:
        expected = hashlib.sha256(archive.read('aapt2')).hexdigest()
    if digest(folder/'aapt2') != expected:
        raise ValueError('Pinned AAPT2 executable checksum mismatch')
    return folder/'bundletool.jar', folder/'aapt2'


def install(output, host=None):
    host = host or host_name()
    if host not in AAPT2:
        raise ValueError('Unsupported AAB tool host')
    output.mkdir()  # Preserve any pre-existing directory or link.
    try:
        urls = {
            'bundletool.jar': f'https://github.com/google/bundletool/releases/download/{BUNDLETOOL_VERSION}/bundletool-all-{BUNDLETOOL_VERSION}.jar',
            'aapt2.jar': f'https://dl.google.com/dl/android/maven2/com/android/tools/build/aapt2/{AAPT2_VERSION}/aapt2-{AAPT2_VERSION}-{AAPT2[host][0]}.jar',
        }
        for name, url in urls.items():
            with urllib.request.urlopen(url, timeout=60) as source, (output/name).open('xb') as target:
                shutil.copyfileobj(source, target)
        if digest(output/'bundletool.jar') != BUNDLETOOL_SHA256 or digest(output/'aapt2.jar') != AAPT2[host][1]:
            raise ValueError('Pinned AAB tool archive checksum mismatch')
        with zipfile.ZipFile(output/'aapt2.jar') as archive:
            (output/'aapt2').write_bytes(archive.read('aapt2'))
            (output/'NOTICE').write_bytes(archive.read('NOTICE'))
        (output/'aapt2').chmod(0o755)
        validate_tools(output, host)
        (output/'provenance.json').write_text(json.dumps(dict(host=host, bundletool=BUNDLETOOL_VERSION, bundletool_sha256=BUNDLETOOL_SHA256, aapt2=AAPT2_VERSION, aapt2_sha256=AAPT2[host][1]), indent=2)+'\n')
    except BaseException:
        shutil.rmtree(output)
        raise


def build(run, tools, sdk, work, manifest, library, files):
    jar, aapt2 = validate_tools(tools)
    compiled = work/'protobuf.zip'
    run([aapt2, 'link', '--proto-format', '--manifest', manifest, '-I', sdk/'platforms/android-34/android.jar', '-o', compiled])
    module = work/'base.zip'
    with zipfile.ZipFile(compiled) as resources, zipfile.ZipFile(module, 'w') as target:
        target.writestr('manifest/AndroidManifest.xml', resources.read('AndroidManifest.xml'))
        if 'resources.pb' in resources.namelist():
            target.writestr('resources.pb', resources.read('resources.pb'))
        target.write(library, 'lib/arm64-v8a/libgame.so')
        for path, name in files:
            target.write(path, 'assets/'+name)
        target.writestr('assets/malvado-files.txt', ''.join(name+'\n' for _, name in files))
    bundle = work/'game.aab'
    run(['java', '-jar', jar, 'build-bundle', '--modules='+str(module), '--output='+str(bundle)])
    run(['java', '-jar', jar, 'validate', '--bundle='+str(bundle)])
    return bundle


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--host', choices=tuple(AAPT2))
    args = parser.parse_args()
    install(args.output, args.host)
