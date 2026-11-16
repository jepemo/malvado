#!/usr/bin/env python3
"""Test Android archive integrity and extraction boundaries without downloads."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
from pathlib import Path
import stat
import sys
import tempfile
import unittest
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

    def test_existing_destination_is_preserved(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "marker").write_text("owned elsewhere")
            with self.assertRaises(ValueError):
                android_archives.prepare(root)
            self.assertEqual((root / "marker").read_text(), "owned elsewhere")


if __name__ == "__main__":
    unittest.main()
