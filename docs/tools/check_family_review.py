#!/usr/bin/env python3
"""Check the alpha.6 family review against the reference catalog."""

import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = re.compile(r"`(bennugd[12])/([\w]+)/([A-Z0-9_]+)`")


def rows(document, heading):
    body = document.split(heading, 1)[1].split("\n\n", 1)[0]
    return [line for line in body.splitlines() if line.startswith("| ") and not line.startswith("| ---")]


def check(compatibility, review, catalog):
    families = [line.split("|")[1].strip() for line in rows(compatibility, "| Family | Target | Minimum example | Difference to verify |")]
    reviewed = rows(review, "| Family | Pinned reference examples | Delivered scope and remaining decision |")
    reference_names = {(record["engine"], record["module"], record["name"]) for record in catalog["records"]
                       if record["kind"] == "function"}
    errors = []
    seen = set()
    for row in reviewed:
        family = row.split("|")[1].strip()
        if family in seen:
            errors.append("duplicate family: " + family)
        seen.add(family)
        references = REFERENCE.findall(row)
        if not references:
            errors.append("missing reference: " + family)
        for reference in references:
            if reference not in reference_names:
                errors.append("unknown reference: " + "/".join(reference))
        if "Unresolved:" not in row:
            errors.append("missing remaining decision: " + family)
    for family in sorted(set(families) - seen):
        errors.append("missing family: " + family)
    for family in sorted(seen - set(families)):
        errors.append("extra family: " + family)
    return errors


def main():
    docs = ROOT / "docs/api"
    compatibility = (docs / "compatibility.md").read_text(encoding="utf-8")
    review = (docs / "family-review.md").read_text(encoding="utf-8")
    catalog = json.loads((docs / "catalog.json").read_text(encoding="utf-8"))
    errors = check(compatibility, review, catalog)
    for error in errors:
        print(error)
    return int(bool(errors))


if __name__ == "__main__":
    raise SystemExit(main())
