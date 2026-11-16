#!/usr/bin/env python3
"""Reject unsupported Rust APK modes before touching tools or an emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import contextlib
import io
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class RustPackageTests(unittest.TestCase):
    def test_unsupported_modes_never_start_build_or_device_work(self):
        base = ['test_android_window.py', '--sdk', 'unused', '--serial', 'emulator-5580', '--language', 'rust']
        for extra in ([], ['--installed-sdk', '--audio'], ['--installed-sdk', '--lifecycle', 'touch'],
                      ['--installed-sdk', '--entry', 'declarative'], ['--installed-sdk', '--failure', 'process']):
            with self.subTest(extra=extra), patch.object(sys, 'argv', base + extra), patch.object(verifier, 'preflight') as preflight, contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as error:
                    verifier.main()
                self.assertEqual(error.exception.code, 2)
                preflight.assert_not_called()


if __name__ == '__main__':
    unittest.main()
