#!/usr/bin/env python3
"""Verify and relocate one SDK archive without accepting stale output."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
from pathlib import Path
import re
import shutil
import tarfile


def inspect_archive(archive):
    archive = Path(archive)
    with archive.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    if Path(str(archive) + '.sha256').read_text() != f'{digest}  {archive.name}\n':
        raise ValueError('SDK checksum does not match')
    with tarfile.open(archive, 'r:gz') as source:
        members = source.getmembers()
        metadata = [entry for entry in members if entry.name.removeprefix('./') ==
                    'share/malvado/malvado-sdk-info.txt']
        if len(metadata) != 1 or not metadata[0].isfile() or metadata[0].size > 4096:
            raise ValueError('SDK metadata is missing or invalid')
        lines = source.extractfile(metadata[0]).read().decode().splitlines()
        values = dict(line.split('=', 1) for line in lines)
        if len(lines) != 3 or set(values) != {'version', 'system', 'architecture'} or any(
                not re.fullmatch(r'[A-Za-z0-9_.-]+', value) for value in values.values()):
            raise ValueError('SDK metadata fields are invalid')
        expected = 'malvado-sdk-{version}-{system}-{architecture}.tar.gz'.format(**values)
        if archive.name != expected:
            raise ValueError('SDK metadata does not match archive name')
        for member in members:
            # Apply the same path/link checks used during relocation.
            if member.name.startswith(('/', '\\')) or '\\' in member.name:
                raise ValueError('SDK archive path is invalid')
            tarfile.data_filter(member, '/malvado-sdk-inspection')
    return dict(values, sha256=digest, archive=archive.name)


def relocate(archive, destination):
    facts = inspect_archive(archive)
    destination = Path(destination)
    destination.mkdir()  # Exclusive ownership; never remove an existing SDK.
    try:
        with tarfile.open(archive, 'r:gz') as source:
            source.extractall(destination, filter='data')
    except BaseException:
        shutil.rmtree(destination)
        raise
    return facts
