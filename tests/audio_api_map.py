#!/usr/bin/env python3
"""Verify that the audio inventory check reports missing and stale names."""

from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "docs/tools"))
from check_audio_api_map import compare


class AudioMapTest(unittest.TestCase):
    def test_detects_missing_and_stale_names(self):
        header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
        inventory = (ROOT / "docs/api/audio-inventory.md").read_text(encoding="utf-8")
        self.assertTrue(all(not item for item in compare(header, inventory)))
        changed = inventory.replace("`mv_music_update`", "`mv_music_removed`")
        errors = compare(header, changed)
        self.assertIn("mv_music_update", errors[0])
        self.assertIn("mv_music_removed", errors[1])
        changed = inventory.replace("`MvSound`", "`MvRemoved`")
        errors = compare(header, changed)
        self.assertIn("MvSound", errors[2])
        self.assertIn("MvRemoved", errors[3])


if __name__ == "__main__":
    unittest.main()
