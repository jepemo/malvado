#!/usr/bin/env python3
"""Reject partial, mismatched and corrupt multiplatform SDK candidates."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from collect_platform_sdk import collect


class CollectionTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.desktop, self.windows = self.root / 'desktop', self.root / 'windows'
        self.desktop.mkdir()
        self.windows.mkdir()
        self.report, self.output = self.root / 'report.json', self.root / 'output'
        self.evidence = {'sha': 'a' * 40, 'runs': [dict(workflow=name, sha='a' * 40, conclusion='success')
                         for name in ('ci.yml', 'windows.yml', 'android.yml')]}
        self.report.write_text(json.dumps(self.evidence))
        self.package(self.desktop, 'darwin', 'arm64')
        self.package(self.desktop, 'linux', 'x86_64')
        self.package(self.windows, 'windows', 'AMD64')

    def package(self, folder, system, arch, version='2-test'):
        archive = folder / f'malvado-sdk-{version}-{system}-{arch}.tar.gz'
        data = f'version={version}\nsystem={system}\narchitecture={arch}\n'.encode()
        with tarfile.open(archive, 'w:gz') as target:
            entry = tarfile.TarInfo('share/malvado/malvado-sdk-info.txt')
            entry.size = len(data)
            target.addfile(entry, io.BytesIO(data))
        digest = hashlib.sha256(archive.read_bytes()).hexdigest()
        Path(str(archive) + '.sha256').write_text(f'{digest}  {archive.name}\n')
        return archive

    def collect(self):
        return collect(self.desktop, self.windows, self.report, self.output)

    def test_collects_three_packages_and_provenance(self):
        result = self.collect()
        self.assertEqual(len(result['sdks']), 3)
        self.assertEqual(len(list(self.output.iterdir())), 7)
        self.assertEqual(json.loads((self.output / 'platform-candidate.json').read_text())['sha'], 'a' * 40)

    def test_missing_windows_and_wrong_target_fail(self):
        for path in self.windows.iterdir():
            path.unlink()
        with self.assertRaises(ValueError):
            self.collect()
        self.package(self.windows, 'linux', 'x86_64')
        with self.assertRaises(ValueError):
            self.collect()
        self.assertFalse(self.output.exists())

    def test_different_versions_fail(self):
        for path in self.windows.iterdir():
            path.unlink()
        self.package(self.windows, 'windows', 'AMD64', '3-test')
        with self.assertRaisesRegex(ValueError, 'versions'):
            self.collect()

    def test_mismatched_revision_fails(self):
        self.evidence['runs'][0]['sha'] = 'b' * 40
        self.report.write_text(json.dumps(self.evidence))
        with self.assertRaisesRegex(ValueError, 'same-revision'):
            self.collect()

    def test_corrupt_archive_fails(self):
        next(self.windows.glob('*.tar.gz')).write_bytes(b'corrupt')
        with self.assertRaisesRegex(ValueError, 'checksum'):
            self.collect()
        self.assertFalse(self.output.exists())

    def test_existing_candidate_preserved_and_failed_copy_cleaned(self):
        self.output.mkdir()
        (self.output / 'keep').write_text('keep')
        with self.assertRaises(FileExistsError):
            self.collect()
        (self.output / 'keep').unlink()
        self.output.rmdir()
        with patch('collect_platform_sdk.shutil.copyfile', side_effect=OSError('disk full')):
            with self.assertRaises(OSError):
                self.collect()
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
