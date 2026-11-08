#!/usr/bin/env python3
"""List public C function declarations from the installed headers."""

import argparse
import difflib
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HEADERS = ROOT / "src/include/malvado"
OUTPUT = ROOT / "docs/api/current-c-api.md"
COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.DOTALL)
FUNCTION = re.compile(r'\b(mv_[a-z][a-z0-9_]*)\s*\(')


def inventory():
    lines = [
        "# Delivered C function inventory",
        "",
        "Generated from the public headers by `python3 docs/tools/current_c_api.py`. ",
        "This is a lexical list of function names, not a complete 2.0 API decision. ",
        "It does not certify contracts, examples, tests, types, constants, or Lua parity. ",
        "See the linked headers and the [process](process-inventory.md), ",
        "[asset](asset-inventory.md), [input](input-inventory.md), ",
        "[audio](audio-inventory.md), and [collision](collision.md) guides.",
        "",
    ]
    for header in sorted(HEADERS.glob("*.h")):
        contents = COMMENT.sub("", header.read_text(encoding="utf-8"))
        names = sorted(set(FUNCTION.findall(contents)))
        lines += [f"## [{header.name}](../../src/include/malvado/{header.name})", ""]
        lines += [f"- `{name}`" for name in names]
        lines.append("")
    return "\n".join(line.rstrip() for line in lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the inventory is out of date")
    args = parser.parse_args()
    expected = inventory()
    if args.check:
        actual = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if actual != expected:
            print("".join(difflib.unified_diff(actual.splitlines(True), expected.splitlines(True),
                                               fromfile=str(OUTPUT), tofile="expected")))
            print("Run python3 docs/tools/current_c_api.py to update the inventory.")
            return 1
    else:
        OUTPUT.write_text(expected, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
