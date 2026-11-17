#!/usr/bin/env python3
"""Exercise SDK archive integrity, metadata and relocation ownership."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import sdk_artifact


class ArtifactTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.work = Path(self.temporary.name)
        self.archive = self.work / 'malvado-sdk-2.0-test-windows-AMD64.tar.gz'

    def package(self, metadata=None, extra=None):
        data = metadata or b'version=2.0-test\nsystem=windows\narchitecture=AMD64\n'
        with tarfile.open(self.archive, 'w:gz') as output:
            entry = tarfile.TarInfo('./share/malvado/malvado-sdk-info.txt')
            entry.size = len(data)
            output.addfile(entry, io.BytesIO(data))
            if extra:
                output.addfile(extra)
        self.checksum()

    def checksum(self):
        digest = hashlib.sha256(self.archive.read_bytes()).hexdigest()
        Path(str(self.archive) + '.sha256').write_text(f'{digest}  {self.archive.name}\n')

    def test_valid_archive_relocates_to_path_with_spaces(self):
        self.package()
        dest = self.work / 'relocated sdk'
        self.assertEqual(sdk_artifact.relocate(self.archive, dest)['system'], 'windows')
        self.assertTrue((dest / 'share/malvado/malvado-sdk-info.txt').is_file())

    def test_checksum_rejects_tampering_before_destination_creation(self):
        self.package()
        self.archive.write_bytes(self.archive.read_bytes() + b'changed')
        dest = self.work / 'sdk'
        with self.assertRaisesRegex(ValueError, 'checksum'):
            sdk_artifact.relocate(self.archive, dest)
        self.assertFalse(dest.exists())

    def test_failed_extraction_removes_only_owned_destination(self):
        self.package()
        dest = self.work / 'sdk'
        with patch.object(tarfile.TarFile, 'extractall', side_effect=OSError('disk full')):
            with self.assertRaises(OSError):
                sdk_artifact.relocate(self.archive, dest)
        self.assertFalse(dest.exists())
        self.assertTrue(self.archive.exists())

    def test_existing_destination_is_preserved(self):
        self.package()
        dest = self.work / 'sdk'
        dest.mkdir()
        (dest / 'keep').write_text('owned by user')
        with self.assertRaises(FileExistsError):
            sdk_artifact.relocate(self.archive, dest)
        self.assertEqual((dest / 'keep').read_text(), 'owned by user')

    def test_wrong_metadata_and_duplicate_keys_are_rejected(self):
        for data in (b'version=other\nsystem=windows\narchitecture=AMD64\n',
                     b'version=2.0-test\nsystem=windows\narchitecture=AMD64\nversion=2.0-test\n'):
            self.package(data)
            with self.assertRaises(ValueError):
                sdk_artifact.inspect_archive(self.archive)

    def test_escaping_paths_and_links_are_rejected(self):
        for path, target in (('../outside', None), ('/absolute', None), ('link', '../../outside')):
            entry = tarfile.TarInfo(path)
            if target:
                entry.type = tarfile.SYMTYPE
                entry.linkname = target
            self.package(extra=entry)
            with self.subTest(path=path), self.assertRaises((ValueError, tarfile.FilterError)):
                sdk_artifact.relocate(self.archive, self.work / 'sdk')
            self.assertFalse((self.work / 'sdk').exists())


if __name__ == '__main__':
    unittest.main()
