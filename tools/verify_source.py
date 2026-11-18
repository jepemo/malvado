#!/usr/bin/env python3
"""Build and test an exact Malvado source archive outside Git."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import platform
import re
import shlex
import subprocess
import tarfile
import tempfile


def extract_source(archive, sha, destination):
    root = f'malvado-source-{sha}'
    if not re.fullmatch(r'[0-9a-f]{40}|[0-9a-f]{64}', sha):
        raise ValueError('expected a full commit ID')
    if archive.name != f'{root}.tar.gz':
        raise ValueError('archive name does not match the expected revision')
    with archive.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        checksum = archive.with_name(archive.name + '.sha256').read_bytes()
        if checksum != f'{digest}  {archive.name}\n'.encode('ascii'):
            raise ValueError('source checksum does not match')
        stream.seek(0)
        with tarfile.open(fileobj=stream, mode='r:gz') as package:
            if package.pax_headers.get('comment') != sha:
                raise ValueError('archive metadata does not match the expected revision')
            paths = set()
            for member in package.getmembers():
                path = PurePosixPath(member.name)
                if (not path.parts or path.parts[0] != root or '\\' in member.name
                        or '..' in path.parts or '.git' in path.parts
                        or path in paths or not (member.isfile() or member.isdir())):
                    raise ValueError(f'invalid source archive entry: {member.name}')
                paths.add(path)
            package.extractall(destination, filter='data')
    source = destination / root
    if (source / 'SOURCE-REVISION').read_bytes() != f'{sha}\n'.encode('ascii'):
        raise ValueError('SOURCE-REVISION does not match the expected revision')
    return source, digest


def run(work, *command):
    print(shlex.join(map(str, command)), flush=True)
    subprocess.run(command, cwd=work, check=True, timeout=600)


def verify_builds(source, work):
    core = work / 'core'
    full = work / 'full'
    sdk = work / 'installed sdk'
    run(work, 'cmake', '-S', source, '-B', core, '-DCMAKE_BUILD_TYPE=Debug',
        '-DMALVADO_BUILD_LUA=OFF', '-DMALVADO_BUILD_RAYLIB=OFF',
        '-DMALVADO_BUILD_ASSETS=OFF', '-DFETCHCONTENT_FULLY_DISCONNECTED=ON')
    run(work, 'cmake', '--build', core, '--config', 'Debug', '--parallel')
    run(work, 'ctest', '--test-dir', core, '-C', 'Debug',
        '--output-on-failure', '--no-tests=error')
    run(work, 'cmake', '-S', source, '-B', full, '-DCMAKE_BUILD_TYPE=Debug',
        '-DMALVADO_BUILD_LUA=ON', '-DMALVADO_BUILD_RAYLIB=ON',
        '-DMALVADO_BUILD_ASSETS=ON', '-DCMAKE_INSTALL_DATADIR=share')
    run(work, 'cmake', '--build', full, '--config', 'Debug', '--parallel')
    run(work, 'cmake', '--install', full, '--prefix', sdk, '--config', 'Debug')
    run(work, 'ctest', '--test-dir', full, '-C', 'Debug',
        '-R', '^installed_core_sdk$', '--output-on-failure', '--no-tests=error')

    notices = sdk / 'share' / 'licenses' / 'malvado'
    originals = {
        'LICENSE': source / 'LICENSE',
        'cJSON-LICENSE': full / '_deps' / 'cjson-src' / 'LICENSE',
        'raylib-LICENSE': full / '_deps' / 'raylib-src' / 'LICENSE',
        'Lua-readme.html': full / '_deps' / 'lua-src' / 'doc' / 'readme.html',
    }
    for name, original in originals.items():
        if (notices / name).read_bytes() != original.read_bytes():
            raise ValueError(f'installed notice differs from its source: {name}')
    return list(originals)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('revision', help='expected full commit ID')
    parser.add_argument('--report', type=Path, required=True, help='new JSON evidence file')
    args = parser.parse_args()
    try:
        with tempfile.TemporaryDirectory(prefix='malvado-source-check-') as directory:
            work = Path(directory)
            source, digest = extract_source(args.archive, args.revision, work)
            notices = verify_builds(source, work)
        result = {
            'revision': args.revision,
            'archive': args.archive.name,
            'sha256': digest,
            'system': platform.system(),
            'architecture': platform.machine(),
            'core_tests': 'passed',
            'installed_c_sdk': 'passed',
            'verified_notices': notices,
        }
        with args.report.open('x', encoding='utf-8') as report:
            json.dump(result, report, indent=2)
            report.write('\n')
        print(f'Source archive verified. Evidence: {args.report}')
    except (OSError, ValueError, subprocess.SubprocessError, tarfile.TarError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
