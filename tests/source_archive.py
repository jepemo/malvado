#!/usr/bin/env python3
"""Check source archive integrity and revision validation."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import hashlib
import io
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from verify_source import extract_source


class SourceArchiveTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.work = Path(temporary.name)
        self.sha = 'a' * 40
        self.root = f'malvado-source-{self.sha}'
        self.archive = self.work / f'{self.root}.tar.gz'
        self.destination = self.work / 'extracted tree'

    def package(self, comment=None, marker=None, extra=None):
        data = f'{marker or self.sha}\n'.encode('ascii')
        with tarfile.open(self.archive, 'w:gz',
                          pax_headers={'comment': comment or self.sha}) as output:
            entry = tarfile.TarInfo(f'{self.root}/SOURCE-REVISION')
            entry.size = len(data)
            output.addfile(entry, io.BytesIO(data))
            if extra:
                output.addfile(tarfile.TarInfo(extra))
        digest = hashlib.sha256(self.archive.read_bytes()).hexdigest()
        self.archive.with_name(self.archive.name + '.sha256').write_bytes(
            f'{digest}  {self.archive.name}\n'.encode('ascii'))
        return digest

    def test_valid_archive(self):
        expected = self.package()
        source, digest = extract_source(self.archive, self.sha, self.destination)
        self.assertEqual(digest, expected)
        self.assertEqual((source / 'SOURCE-REVISION').read_text().strip(), self.sha)

    def test_changed_bytes_fail_checksum(self):
        self.package()
        with self.archive.open('ab') as stream:
            stream.write(b'changed')
        with self.assertRaisesRegex(ValueError, 'checksum'):
            extract_source(self.archive, self.sha, self.destination)
        self.assertFalse(self.destination.exists())

    def test_revision_must_match_both_records(self):
        for field in ('comment', 'marker'):
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'revision'):
                self.package(**{field: 'b' * 40})
                extract_source(self.archive, self.sha, self.destination)

    def test_invalid_paths_are_rejected_before_extraction(self):
        for path in (f'{self.root}/../outside', f'{self.root}/.git/config',
                     f'{self.root}/SOURCE-REVISION'):
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'entry'):
                self.package(extra=path)
                extract_source(self.archive, self.sha, self.destination)
        self.assertFalse(self.destination.exists())


if __name__ == '__main__':
    unittest.main()
