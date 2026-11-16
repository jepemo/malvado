#!/usr/bin/env python3
"""Check the APK fixture payload before expensive emulator execution."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import sys
import unittest
import zipfile
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from test_android_window import ROOT, add_lua_assets


class LuaPackageTests(unittest.TestCase):
    def test_entry_selection_and_binary_bytes(self):
        for entry in ('explicit', 'declarative'):
            for failure in ('none', 'process', 'missing-image'):
                with self.subTest(entry=entry, failure=failure):
                    payload = io.BytesIO()
                    with zipfile.ZipFile(payload, 'w') as archive:
                        add_lua_assets(archive, entry, failure)
                    with zipfile.ZipFile(payload) as archive:
                        names = set(archive.namelist())
                        self.assertEqual(len(names), len(archive.namelist()))
                        self.assertEqual(archive.read('assets/message.bin'), b'\0\r\n\xff')
                        self.assertEqual(archive.read('assets/main.lua'),
                                         (ROOT / 'tests/android-lua' / (entry + '.lua')).read_bytes())
                        self.assertIn('assets/atlas.json', names)
                        self.assertIn('assets/behavior.lua', names)
                        self.assertEqual('assets/red.png' in names, failure != 'missing-image')
                        self.assertTrue(all(name.startswith('assets/') for name in names))

    def test_invalid_modes_do_not_mutate_archive(self):
        with zipfile.ZipFile(io.BytesIO(), 'w') as archive:
            with self.assertRaises(ValueError):
                add_lua_assets(archive, '../other', 'none')
            with self.assertRaises(ValueError):
                add_lua_assets(archive, 'explicit', 'unknown')
            self.assertEqual(archive.namelist(), [])

    def test_keyboard_module_is_packaged_only_when_selected(self):
        for enabled in (False, True):
            payload = io.BytesIO()
            with zipfile.ZipFile(payload, 'w') as archive:
                add_lua_assets(archive, 'explicit', 'none', keyboard=enabled)
            with zipfile.ZipFile(payload) as archive:
                self.assertEqual('assets/keyboard.lua' in archive.namelist(), enabled)
                if enabled:
                    self.assertEqual(archive.read('assets/keyboard.lua'), (ROOT / 'tests/android-lua/keyboard.lua').read_bytes())


if __name__ == '__main__':
    unittest.main()
