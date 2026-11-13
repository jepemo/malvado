#!/usr/bin/env python3
"""Check test-only Mesa deployment and archive rejection without a display."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import windows_graphics as driver


class Driver(unittest.TestCase):
    def test_bad_archive_is_rejected_and_removed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / driver.ARCHIVE
            archive.write_bytes(b"invalid")
            with self.assertRaisesRegex(RuntimeError, "checksum mismatch"):
                driver.prepare(root)
            self.assertFalse(archive.exists())

    def test_only_required_libraries_are_deployed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source, destination = root / "driver", root / "test executable"
            source.mkdir()
            destination.mkdir()
            for name in driver.DRIVER_FILES:
                (source / name).write_bytes(name.encode())
            (source / "unrelated.dll").write_bytes(b"not required")
            with patch.dict(os.environ, MALVADO_MESA_DIR=str(source)):
                driver.deploy(destination)
                driver.deploy(destination)
                for name in driver.DRIVER_FILES:
                    self.assertEqual((destination / name).read_bytes(), name.encode())
                self.assertFalse((destination / "unrelated.dll").exists())
                (source / driver.DRIVER_FILES[0]).unlink()
                with self.assertRaisesRegex(RuntimeError, "incomplete"):
                    driver.deploy(destination)


if __name__ == "__main__":
    unittest.main()
