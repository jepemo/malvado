#!/usr/bin/env python3
"""Check C public enum and constant names against the API inventories."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
ENUM = re.compile(r"typedef\s+enum\s*\{([^{}]*)\}\s*(Mv\w+)\s*;", re.DOTALL)


def compare(headers, maps):
    declared_types, declared_constants = set(), set()
    for header in headers:
        for body, name in ENUM.findall(COMMENT.sub("", header)):
            declared_types.add(name)
            declared_constants.update(re.findall(r"\bMV_[A-Z0-9_]+\b", body))
    tables = [doc.split(heading, 1)[1].split("\n\n", 1)[0] for doc, heading in maps]
    mapped_types = set(re.findall(r"`(Mv\w+)`", "\n".join(tables)))
    mapped_constants = set(re.findall(r"`(MV_[A-Z0-9_]+)`", "\n".join(tables)))
    return (declared_types - mapped_types, mapped_types - declared_types,
            declared_constants - mapped_constants, mapped_constants - declared_constants)


def main():
    headers = [p.read_text(encoding="utf-8") for p in sorted((ROOT / "src/include/malvado").glob("*.h"))]
    docs = ROOT / "docs/api"
    maps = [((docs / file).read_text(encoding="utf-8"), heading) for file, heading in (
        ("process-inventory.md", "| C enum | C constants | Lua relationship |"),
        ("asset-inventory.md", "| C enum | C constants | Lua relationship |"),
        ("collision-inventory.md", "| C enum | C constants | Lua relationship |"),
        ("input-inventory.md", "| C type | C constants | Lua relationship |"))]
    errors = compare(headers, maps)
    for label, names in zip(("missing enums", "extra enums", "missing constants", "extra constants"), errors):
        if names:
            print(f"public enums: {label}: {', '.join(sorted(names))}")
    return int(any(errors))


if __name__ == "__main__":
    raise SystemExit(main())
