#!/usr/bin/env python3
"""Test Android archive integrity and extraction boundaries without downloads."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
import json
import os
from pathlib import Path
import stat
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import android_archives


class ArchiveTests(unittest.TestCase):
    def test_integrity_paths_and_executable_mode(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "test.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                entry = zipfile.ZipInfo("emulator/emulator")
                entry.external_attr = (stat.S_IFREG | 0o755) << 16
                bundle.writestr(entry, b"binary")
            with self.assertRaisesRegex(ValueError, "SHA-256"):
                android_archives.extract(archive, root / "bad", "0" * 64)
            self.assertFalse((root / "bad").exists())
            android_archives.extract(archive, root / "good", hashlib.sha256(archive.read_bytes()).hexdigest())
            target = root / "good/emulator/emulator"
            self.assertEqual(target.read_bytes(), b"binary")
            if os.name != "nt":
                self.assertTrue(target.stat().st_mode & stat.S_IXUSR)

    def test_traversal_and_symlinks_are_rejected_before_writes(self):
        for name, mode in (("../outside", stat.S_IFREG), ("/absolute", stat.S_IFREG),
                           ("redirect", stat.S_IFLNK)):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                archive = root / "test.zip"
                with zipfile.ZipFile(archive, "w") as bundle:
                    bundle.writestr("first", b"safe")
                    entry = zipfile.ZipInfo(name)
                    entry.external_attr = mode << 16
                    bundle.writestr(entry, b"../outside")
                with self.assertRaises(ValueError):
                    android_archives.extract(archive, root / "output", hashlib.sha256(archive.read_bytes()).hexdigest())
                self.assertFalse((root / "output").exists())

    def test_download_failure_removes_only_owned_staging(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            marker = root / "unrelated"
            marker.write_text("keep")
            with patch.object(android_archives.urllib.request, "urlopen", side_effect=OSError("offline")):
                with self.assertRaises(OSError):
                    android_archives.prepare(root / "result")
            self.assertEqual(list(root.iterdir()), [marker])
            self.assertEqual(marker.read_text(), "keep")

    @unittest.skipIf(os.name == 'nt', 'NDK file links are a Linux SDK input')
    def test_ndk_file_links_and_chains(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / 'ndk.zip'
            with zipfile.ZipFile(archive, 'w') as bundle:
                bundle.writestr('ndk/lib/file', b'native library')
                for name, target in [('ndk/lib/alias', 'file'), ('ndk/bin/alias', '../lib/alias')]:
                    entry = zipfile.ZipInfo(name)
                    entry.external_attr = (stat.S_IFLNK | 0o777) << 16
                    bundle.writestr(entry, target)
            android_archives.extract(archive, root / 'sdk', hashlib.sha256(archive.read_bytes()).hexdigest(), file_symlinks=True)
            self.assertTrue((root / 'sdk/ndk/bin/alias').is_symlink())
            self.assertEqual((root / 'sdk/ndk/bin/alias').read_bytes(), b'native library')

    def test_ndk_unsafe_links_are_rejected_before_writes(self):
        for target, child in [('/outside', False), ('../../outside', False), ('missing', False),
                              ('link', False), ('directory', False), ('file', True)]:
            with self.subTest(target=target, child=child), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                archive = root / 'ndk.zip'
                with zipfile.ZipFile(archive, 'w') as bundle:
                    bundle.writestr('ndk/file', b'contents')
                    bundle.writestr('ndk/directory/', b'')
                    link = zipfile.ZipInfo('ndk/link')
                    link.external_attr = (stat.S_IFLNK | 0o777) << 16
                    bundle.writestr(link, target)
                    if child:
                        bundle.writestr('ndk/link/child', b'escape')
                with self.assertRaises(ValueError):
                    android_archives.extract(archive, root / 'sdk', hashlib.sha256(archive.read_bytes()).hexdigest(), file_symlinks=True)
                self.assertFalse((root / 'sdk').exists())

    def test_macos_archive_layout_integrity_and_provenance(self):
        for valid in (True, False):
            with self.subTest(valid=valid), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                data = io.BytesIO()
                with zipfile.ZipFile(data, 'w') as bundle:
                    for name in ('emulator/emulator', 'emulator/source.properties',
                                 ('arm64-v8a' if valid else 'x86_64') + '/system.img',
                                 ('arm64-v8a' if valid else 'x86_64') + '/source.properties'):
                        bundle.writestr(name, 'fixture')
                raw = data.getvalue()
                archives = [('macos.zip', hashlib.sha256(raw).hexdigest())]
                with patch.object(android_archives, 'MACOS_ARCHIVES', archives), patch.object(android_archives.urllib.request, 'urlopen', return_value=io.BytesIO(raw)):
                    if valid:
                        android_archives.prepare(root / 'runtime', 'macos')
                        provenance = json.loads((root / 'runtime/malvado-emulator-archives.json').read_text())
                        self.assertEqual(provenance, dict(host='macos', abi='arm64-v8a', archives=[dict(archive=archives[0][0], sha256=archives[0][1])]))
                    else:
                        with self.assertRaisesRegex(ValueError, 'Incomplete'):
                            android_archives.prepare(root / 'runtime', 'macos')
                        self.assertEqual(list(root.iterdir()), [])

    def test_default_linux_archives_keep_x86_64_and_record_provenance(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'runtime'
            data = io.BytesIO()
            with zipfile.ZipFile(data, 'w') as bundle:
                for name in ('emulator/emulator', 'x86_64/system.img', 'x86_64/source.properties'):
                    bundle.writestr(name, 'fixture')
            raw = data.getvalue()
            archives = [('linux.zip', hashlib.sha256(raw).hexdigest())]
            with patch.object(android_archives, 'ARCHIVES', archives), patch.object(android_archives.urllib.request, 'urlopen', return_value=io.BytesIO(raw)):
                android_archives.prepare(output)
            provenance = json.loads((output / 'malvado-emulator-archives.json').read_text())
            self.assertEqual(provenance['host'], 'linux')
            self.assertEqual(provenance['abi'], 'x86_64')
            self.assertEqual(provenance['archives'][0]['archive'], 'linux.zip')

    def test_invalid_host_fails_before_download_and_staging(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'new parent/runtime'
            with patch.object(android_archives.urllib.request, 'urlopen') as download:
                with self.assertRaisesRegex(ValueError, 'host'):
                    android_archives.prepare(output, 'windows')
                download.assert_not_called()
            self.assertFalse(output.parent.exists())

    @unittest.skipIf(os.name == 'nt', 'Creating symlinks needs Windows privileges')
    def test_broken_destination_link_is_preserved(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'runtime'
            output.symlink_to('missing')
            with patch.object(android_archives.urllib.request, 'urlopen') as download:
                with self.assertRaisesRegex(ValueError, 'already exists'):
                    android_archives.prepare(output, 'macos')
                download.assert_not_called()
            self.assertTrue(output.is_symlink())

    def test_existing_destination_is_preserved(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "marker").write_text("owned elsewhere")
            with self.assertRaises(ValueError):
                android_archives.prepare(root)
            self.assertEqual((root / "marker").read_text(), "owned elsewhere")


if __name__ == "__main__":
    unittest.main()
