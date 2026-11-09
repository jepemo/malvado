#!/usr/bin/env python3
"""Check that the process API map covers all functions in malvado.h."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
header = (ROOT / "src/include/malvado/malvado.h").read_text(encoding="utf-8")
header = re.sub(r"/\*.*?\*/|//[^\n]*", "", header, flags=re.DOTALL)
names = set(re.findall(r"\b(mv_[a-z][a-z0-9_]*)\s*\(", header))
text = (ROOT / "docs/api/process-inventory.md").read_text(encoding="utf-8")
table = text.split("| Area | C baseline | Lua baseline |", 1)[1].split("\n\n", 1)[0]
mapped = set(re.findall(r"`(mv_[a-z][a-z0-9_]*)`", table))
missing = sorted(names - mapped)
extra = sorted(mapped - names)
if missing or extra:
    if missing:
        print("Missing from process API map: " + ", ".join(missing))
    if extra:
        print("Not declared in malvado.h: " + ", ".join(extra))
    raise SystemExit(1)
