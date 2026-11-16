#!/usr/bin/env python3
"""Test isolated SDK publication and cleanup without downloading Android tools."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_sdk


class SdkTests(unittest.TestCase):
    def package(self, *, complete=True):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as bundle:
            bundle.writestr('ndk/source.properties', 'Pkg.Revision=27.2.12479018\n')
            if complete:
                bundle.writestr('ndk/toolchain.cmake', 'toolchain')
        return data.getvalue()

    def install(self, output, data, *, digest=None):
        packages = [('test.zip', digest or hashlib.sha256(data).hexdigest(), 'ndk', 'ndk/version', 'toolchain.cmake', False)]
        with patch.object(android_sdk, 'PACKAGES', packages), patch.object(android_sdk.urllib.request, 'urlopen', return_value=io.BytesIO(data)):
            android_sdk.prepare(output)

    def test_install_records_digest_and_preserves_unrelated_files(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'keep').write_text('unrelated')
            data = self.package()
            self.install(root / 'SDK with spaces', data)
            sdk = root / 'SDK with spaces'
            self.assertEqual((sdk / 'ndk/version/toolchain.cmake').read_text(), 'toolchain')
            self.assertEqual(json.loads((sdk / 'malvado-archives.json').read_text()),
                             [dict(archive='test.zip', sha256=hashlib.sha256(data).hexdigest(), destination='ndk/version')])
            self.assertEqual({p.name for p in root.iterdir()}, {'keep', 'SDK with spaces'})

    def test_failure_cleans_staging_without_publishing_sdk(self):
        for complete, digest in [(True, '0' * 64), (False, None)]:
            with self.subTest(complete=complete), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                (root / 'keep').write_text('unrelated')
                with self.assertRaises(ValueError):
                    self.install(root / 'sdk', self.package(complete=complete), digest=digest)
                self.assertEqual([p.name for p in root.iterdir()], ['keep'])

    def test_download_failure_and_existing_destinations(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with patch.object(android_sdk.urllib.request, 'urlopen', side_effect=OSError('offline')):
                with self.assertRaises(OSError):
                    android_sdk.prepare(root / 'sdk')
            self.assertEqual(list(root.iterdir()), [])
            (root / 'sdk').mkdir()
            (root / 'sdk/keep').write_text('pre-existing')
            with self.assertRaises(ValueError):
                android_sdk.prepare(root / 'sdk')
            self.assertEqual((root / 'sdk/keep').read_text(), 'pre-existing')


if __name__ == '__main__':
    unittest.main()
