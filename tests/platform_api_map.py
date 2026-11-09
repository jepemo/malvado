#!/usr/bin/env python3
"""Check coverage and drift detection in the platform API inventory."""

from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "docs/tools"))
from check_platform_api_map import compare


class PlatformMapTest(unittest.TestCase):
    def test_names_and_new_unrelated_declarations(self):
        header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
        platform, inputs, audio = [(ROOT / "docs/api" / name).read_text(encoding="utf-8")
                                   for name in ("platform-inventory.md", "input-inventory.md", "audio-inventory.md")]
        self.assertTrue(all(not item for item in compare(header, platform, inputs, audio)))
        extra_header = header + "\nint mv_new_service(void);\ntypedef struct MvNew MvNew;\n"
        errors = compare(extra_header, platform, inputs, audio)
        self.assertIn("mv_new_service", errors[0])
        self.assertIn("MvNew", errors[2])
        stale = platform.replace("`mv_window_open`", "`mv_removed`")
        errors = compare(header, stale, inputs, audio)
        self.assertIn("mv_window_open", errors[0])
        self.assertIn("mv_removed", errors[1])


if __name__ == "__main__":
    unittest.main()
