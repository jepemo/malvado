#!/usr/bin/env python3
"""Check enum inventory drift detection."""

from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "docs/tools"))
from check_public_enum_map import compare


class EnumMapTest(unittest.TestCase):
    def test_detects_missing_and_stale_names(self):
        headers = [p.read_text(encoding="utf-8") for p in (ROOT / "src/include/malvado").glob("*.h")]
        docs = ROOT / "docs/api"
        maps = [((docs / name).read_text(encoding="utf-8"), heading) for name, heading in (
            ("process-inventory.md", "| C enum | C constants | Lua relationship |"),
            ("collision-inventory.md", "| C enum | C constants | Lua relationship |"),
            ("input-inventory.md", "| C type | C constants | Lua relationship |"))]
        self.assertTrue(all(not item for item in compare(headers, maps)))
        errors = compare(headers + ["typedef enum { MV_NEW_VALUE } MvNewEnum;"], maps)
        self.assertIn("MvNewEnum", errors[0])
        self.assertIn("MV_NEW_VALUE", errors[2])
        changed = list(maps)
        changed[0] = (changed[0][0].replace("`MV_STEP_DONE`", "`MV_STEP_REMOVED`"), changed[0][1])
        errors = compare(headers, changed)
        self.assertIn("MV_STEP_DONE", errors[2])
        self.assertIn("MV_STEP_REMOVED", errors[3])


if __name__ == "__main__":
    unittest.main()
