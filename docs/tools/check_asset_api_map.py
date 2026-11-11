#!/usr/bin/env python3
"""Check that the asset inventory maps the public asset names."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
FUNCTION = re.compile(r"\b(mv_[a-z][a-z0-9_]*)\s*\(")
TYPE = re.compile(r"\}\s*(Mv\w+)\s*;|typedef\s+struct\s+\w+\s+(Mv\w+)\s*;")


def check(header, function_table, type_table):
    header = COMMENT.sub("", header)
    functions = set(FUNCTION.findall(header))
    types = {a or b for a, b in TYPE.findall(header)}
    mapped_functions = set(re.findall(r"`(mv_[a-z][a-z0-9_]*)`", function_table))
    mapped_types = set(re.findall(r"`(Mv\w+)`", type_table))
    return functions - mapped_functions, mapped_functions - functions, types - mapped_types, mapped_types - types


def main():
    inventory = (ROOT / "docs/api/asset-inventory.md").read_text(encoding="utf-8")
    table = inventory.split("| Area | C | Lua | Evidence |", 1)[1].split("\n\n", 1)[0]
    failures = False
    for name, heading in (("atlas", "Atlas"), ("fpg", "FPG"), ("asset", "Asset")):
        rows = "\n".join(line for line in table.splitlines() if line.startswith(f"| {heading} "))
        type_table = inventory.split(f"## {heading} types and ownership", 1)[1].split("\n\n", 2)[1]
        header = (ROOT / f"src/include/malvado/{name}.h").read_text(encoding="utf-8")
        errors = check(header, rows, type_table)
        for label, values in zip(("missing functions", "extra functions", "missing types", "extra types"), errors):
            if values:
                print(f"{name}: {label}: {', '.join(sorted(values))}")
        failures |= any(errors)
    return int(failures)


if __name__ == "__main__":
    raise SystemExit(main())
