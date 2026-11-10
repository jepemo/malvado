#!/usr/bin/env python3
"""Check detection of missing families and invalid reference names."""

import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "docs/tools"))
from check_family_review import check


class FamilyReviewTest(unittest.TestCase):
    def test_detects_drift(self):
        docs = ROOT / "docs/api"
        compatibility = (docs / "compatibility.md").read_text(encoding="utf-8")
        review = (docs / "family-review.md").read_text(encoding="utf-8")
        catalog = json.loads((docs / "catalog.json").read_text(encoding="utf-8"))
        self.assertEqual(check(compatibility, review, catalog), [])
        changed = review.replace("bennugd2/libmod_misc/GET_ID", "bennugd2/libmod_misc/NOT_A_FUNCTION")
        self.assertIn("unknown reference: bennugd2/libmod_misc/NOT_A_FUNCTION", check(compatibility, changed, catalog))
        unresolved = review.replace("Disposition:", "Unresolved:")
        self.assertTrue(any("missing remaining decision" in error for error in check(compatibility, unresolved, catalog)))
        changed = review.replace("| FLIC/CD-ROM |", "| Removed family |")
        errors = check(compatibility, changed, catalog)
        self.assertIn("missing family: FLIC/CD-ROM", errors)
        self.assertIn("extra family: Removed family", errors)


if __name__ == "__main__":
    unittest.main()
