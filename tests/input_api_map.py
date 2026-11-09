#!/usr/bin/env python3
"""Verify that the input inventory check reports missing and stale names."""

from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "docs/tools"))
from check_input_api_map import compare


class InputMapTest(unittest.TestCase):
    def test_detects_missing_and_stale_names(self):
        header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
        inventory = (ROOT / "docs/api/input-inventory.md").read_text(encoding="utf-8")
        self.assertTrue(all(not item for item in compare(header, inventory)))
        missing = inventory.replace("`mv_key_down`", "`mv_key_removed`")
        errors = compare(header, missing)
        self.assertIn("mv_key_down", errors[0])
        self.assertIn("mv_key_removed", errors[1])
        missing = inventory.replace("`MV_KEY_A`", "`MV_KEY_REMOVED`")
        errors = compare(header, missing)
        self.assertIn("MV_KEY_A", errors[4])
        self.assertIn("MV_KEY_REMOVED", errors[5])


if __name__ == "__main__":
    unittest.main()
