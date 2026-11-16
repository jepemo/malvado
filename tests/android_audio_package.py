#!/usr/bin/env python3
"""Verify APK audio fixtures and keep the C payload independent of Lua."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import sys
import unittest
import zipfile
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from test_android_window import ROOT, add_audio_assets


class AudioPackageTests(unittest.TestCase):
    def test_c_payload_contains_only_media(self):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as archive:
            add_audio_assets(archive)
        with zipfile.ZipFile(data) as archive:
            self.assertEqual(set(archive.namelist()), {'assets/silence.wav', 'assets/silence.ogg', 'assets/empty.wav'})
            self.assertEqual(archive.read('assets/silence.wav')[:4], b'RIFF')
            self.assertEqual(archive.read('assets/silence.wav')[8:12], b'WAVE')
            self.assertEqual(archive.read('assets/silence.ogg')[:4], b'OggS')
            self.assertEqual(len(archive.read('assets/empty.wav')), 44)

    def test_lua_payload_preserves_media_and_behavior_bytes(self):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as archive:
            add_audio_assets(archive, lua=True)
        with zipfile.ZipFile(data) as archive:
            for name in ('silence.wav', 'silence.ogg', 'empty.wav'):
                self.assertEqual(archive.read('assets/' + name), (ROOT / 'tests/fixtures' / name).read_bytes())
            self.assertEqual(archive.read('assets/audio.lua'), (ROOT / 'tests/android-lua/audio.lua').read_bytes())


if __name__ == '__main__':
    unittest.main()
