#!/usr/bin/env python3
"""Collect verified SDK, source and documentation artifacts into a draft."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import tarfile
import tempfile

from release_notes import VERSION, candidate, read_tree
from sdk_artifact import inspect_archive
from verify_source import extract_source


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def collect(platform, source, docs, version, output):
    if not re.fullmatch(VERSION, version):
        raise ValueError('Invalid candidate version')
    report = read_json(platform / 'platform-candidate.json')
    revision = report['sha']
    runs = report['runs']
    if (len(runs) != 3 or {run['workflow'] for run in runs} != {'ci.yml', 'windows.yml', 'android.yml'}
            or any(run['sha'] != revision or run['conclusion'] != 'success' for run in runs)):
        raise ValueError('Complete same-revision platform evidence is required')
    targets = {('darwin', 'arm64'), ('linux', 'x86_64'), ('windows', 'AMD64')}
    sdks = report['sdks']
    if len(sdks) != 3 or {(sdk['system'], sdk['architecture']) for sdk in sdks} != targets:
        raise ValueError('Expected one SDK for each desktop target')

    archives = []
    for system, architecture in sorted(targets):
        archive = platform / f'malvado-sdk-{version}-{system}-{architecture}.tar.gz'
        if inspect_archive(archive) not in sdks:
            raise ValueError(f'SDK evidence does not match {archive.name}')
        archives.append(archive)

    archive = source / 'package' / f'malvado-source-{revision}.tar.gz'
    source_report = read_json(source / 'verification.json')
    with tempfile.TemporaryDirectory(prefix='malvado-candidate-source-') as temporary:
        tree, digest = extract_source(archive, revision, Path(temporary))
        notice = (tree / 'licenses' / 'raylib-NOTICES.txt').read_bytes()
        notes_content = candidate(lambda path: read_tree(tree, path), version, revision).encode('utf-8')
    for sdk in archives:
        with tarfile.open(sdk, 'r:gz') as package:
            path = 'share/licenses/malvado/raylib-NOTICES.txt'
            member = next((entry for entry in package if entry.name.removeprefix('./') == path), None)
            if (member is None or not member.isfile()
                    or package.extractfile(member).read().replace(b'\r\n', b'\n') != notice):
                raise ValueError(f'Invalid dependency notices in {sdk.name}')
    if (source_report['revision'] != revision or source_report['archive'] != archive.name
            or source_report['sha256'] != digest or source_report['core_tests'] != 'passed'
            or source_report['installed_c_sdk'] != 'passed'
            or 'raylib-NOTICES.txt' not in source_report['verified_notices']):
        raise ValueError('Source verification does not match the candidate')
    archives.append(archive)

    expected_docs = {
        'version': version, 'revision': revision, 'path': f'{version}/',
        'release_notes_sha256': hashlib.sha256(notes_content).hexdigest(),
    }
    if (read_json(docs / 'candidate.json') != expected_docs
            or not (docs / version / 'index.html').is_file()
            or not (docs / version / 'docs' / 'changelog' / 'index.html').is_file()):
        raise ValueError('Documentation or release notes do not match the candidate')

    output.mkdir()
    try:
        for archive in archives:
            shutil.copyfile(archive, output / archive.name)
            checksum = archive.with_name(archive.name + '.sha256')
            shutil.copyfile(checksum, output / checksum.name)
        shutil.copyfile(platform / 'platform-candidate.json', output / 'platform-candidate.json')
        shutil.copyfile(source / 'verification.json', output / 'source-verification.json')
        shutil.copytree(docs, output / 'docs')
        (output / 'RELEASE-NOTES.md').write_bytes(notes_content)
        checksums = []
        for file in sorted(output.rglob('*')):
            if file.is_file():
                with file.open('rb') as stream:
                    digest = hashlib.file_digest(stream, 'sha256').hexdigest()
                checksums.append(f'{digest}  {file.relative_to(output).as_posix()}\n')
        (output / 'SHA256SUMS').write_bytes(''.join(checksums).encode('ascii'))
    except BaseException:
        shutil.rmtree(output)
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('platform', 'source', 'docs', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    collect(args.platform, args.source, args.docs, args.version, args.output)
    print(f'Prepared draft candidate at {args.output}')


if __name__ == '__main__':
    main()
