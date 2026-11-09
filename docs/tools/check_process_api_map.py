#!/usr/bin/env python3
"""Check C function and type names in the process API map."""

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
type_pattern = (r"\}\s*(Mv\w+)\s*;|typedef\s+(?:struct\s+\w+|uint64_t)\s+(Mv\w+)\s*;"
                r"|typedef\s+[^;{}]+\(\*(Mv\w+)\)\s*\(")
types = {a or b or c for a, b, c in re.findall(type_pattern, header)}
type_table = text.split("| C types | Meaning and Lua relationship |", 1)[1].split("\n\n", 1)[0]
mapped_types = set(re.findall(r"`(Mv\w+)`", type_table))
missing_types = sorted(types - mapped_types)
extra_types = sorted(mapped_types - types)
if missing or extra or missing_types or extra_types:
    if missing:
        print("Missing from process API map: " + ", ".join(missing))
    if extra:
        print("Not declared in malvado.h: " + ", ".join(extra))
    if missing_types:
        print("Missing types from process API map: " + ", ".join(missing_types))
    if extra_types:
        print("Types not declared in malvado.h: " + ", ".join(extra_types))
    raise SystemExit(1)
