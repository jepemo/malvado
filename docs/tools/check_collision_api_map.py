#!/usr/bin/env python3
"""Check the collision inventory against the public header."""

from pathlib import Path
import sys

sys.dont_write_bytecode = True
from check_asset_api_map import check

ROOT = Path(__file__).resolve().parents[2]
header = (ROOT / "src/include/malvado/collision.h").read_text(encoding="utf-8")
inventory = (ROOT / "docs/api/collision-inventory.md").read_text(encoding="utf-8")
functions = inventory.split("| Area | C | Lua | Evidence |", 1)[1].split("\n\n", 1)[0]
types = inventory.split("## Collision types", 1)[1].split("\n\n", 2)[1]
errors = check(header, functions, types)
for label, values in zip(("missing functions", "extra functions", "missing types", "extra types"), errors):
    if values:
        print(f"collision: {label}: {', '.join(sorted(values))}")
raise SystemExit(int(any(errors)))
