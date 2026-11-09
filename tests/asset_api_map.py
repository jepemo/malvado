#!/usr/bin/env python3
"""Check that asset map validation detects missing and extra names."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "docs/tools"))
from check_asset_api_map import check


class AssetMapTest(unittest.TestCase):
    def test_detects_missing_and_extra(self):
        header = "typedef struct MvAtlas MvAtlas; int mv_atlas_count(void);"
        self.assertEqual(check(header, "`mv_atlas_count`", "`MvAtlas`"),
                         (set(), set(), set(), set()))
        self.assertEqual(check(header, "`mv_other`", "`MvOther`"),
                         ({"mv_atlas_count"}, {"mv_other"}, {"MvAtlas"}, {"MvOther"}))
        self.assertEqual(check(header + " /* int mv_ignored(void); */",
                               "`mv_atlas_count`", "`MvAtlas`"),
                         (set(), set(), set(), set()))


if __name__ == "__main__":
    unittest.main()
