#!/usr/bin/env python3
"""Reject unsupported Rust APK modes before touching tools or an emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import contextlib
import io
from pathlib import Path
import sys
import unittest
import zipfile
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class RustPackageTests(unittest.TestCase):
    def test_rust_payload_has_exact_binary_png_and_atlas_inputs(self):
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            verifier.add_rust_assets(archive)
        with zipfile.ZipFile(payload) as archive:
            self.assertEqual(set(archive.namelist()), {'assets/message.bin', 'assets/atlas.json', 'assets/red.png'})
            self.assertEqual(archive.read('assets/message.bin'), b'\x00\r\n\xff')
            self.assertEqual(archive.read('assets/red.png'), (verifier.ROOT / 'tests/fixtures/red.png').read_bytes())
            self.assertEqual(archive.read('assets/atlas.json'), (verifier.ROOT / 'tests/fixtures/atlas.json').read_bytes())

    def test_missing_image_is_an_intentional_apk_omission(self):
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            verifier.add_rust_assets(archive, 'missing-image')
        with zipfile.ZipFile(payload) as archive:
            self.assertNotIn('assets/red.png', archive.namelist())
            self.assertEqual(archive.read('assets/message.bin'), b'\x00\r\n\xff')
            self.assertIn('assets/atlas.json', archive.namelist())
        with zipfile.ZipFile(io.BytesIO(), 'w') as archive:
            with self.assertRaises(ValueError):
                verifier.add_rust_assets(archive, 'invalid')
            self.assertEqual(archive.namelist(), [])

    def test_rust_audio_payload_keeps_exact_media_bytes_without_lua(self):
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            verifier.add_rust_assets(archive)
            verifier.add_audio_assets(archive)
        with zipfile.ZipFile(payload) as archive:
            self.assertEqual(len(archive.namelist()), 6)
            self.assertFalse(any(name.endswith('.lua') for name in archive.namelist()))
            for name in ('silence.wav', 'silence.ogg', 'empty.wav'):
                self.assertEqual(archive.read('assets/' + name), (verifier.ROOT / 'tests/fixtures' / name).read_bytes())

    def test_unsupported_modes_never_start_build_or_device_work(self):
        base = ['test_android_window.py', '--sdk', 'unused', '--serial', 'emulator-5580', '--language', 'rust']
        for extra in ([], ['--installed-sdk', '--audio', '--lifecycle', 'touch'],
                      ['--installed-sdk', '--failure', 'process', '--lifecycle', 'resume'], ['--installed-sdk', '--entry', 'declarative']):
            with self.subTest(extra=extra), patch.object(sys, 'argv', base + extra), patch.object(verifier, 'preflight') as preflight, contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as error:
                    verifier.main()
                self.assertEqual(error.exception.code, 2)
                preflight.assert_not_called()


if __name__ == '__main__':
    unittest.main()
