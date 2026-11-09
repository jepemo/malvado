#!/usr/bin/env python3
"""Check the documented input API names against the public C header."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
FUNCTION = re.compile(r"\b(mv_[a-z][a-z0-9_]*)\s*\(")
INPUT_PREFIXES = ("mv_key_", "mv_mouse_", "mv_pad_", "mv_text_input_", "mv_utf8_")
INPUT_WINDOW = {"mv_window_escape_down", "mv_window_space_down"}
ENUMS = ("MvKey", "MvMouseButton", "MvPadButton", "MvPadAxis")


def compare(header, inventory):
    header = COMMENT.sub("", header)
    declared = set(FUNCTION.findall(header))
    declared = {name for name in declared if name.startswith(INPUT_PREFIXES) or name in INPUT_WINDOW}
    table = inventory.split("| Area | C | Lua | Examples and automated evidence |", 1)[1].split("\n\n", 1)[0]
    mapped = set(re.findall(r"`(mv_[a-z][a-z0-9_]*)`", table))
    enum_table = inventory.split("## C input types and constants", 1)[1].split("\n\n", 2)[1]
    documented_types = set(re.findall(r"`(Mv\w+)`", enum_table))
    documented_values = set(re.findall(r"`(MV_(?:KEY|MOUSE|PAD)_[A-Z0-9_]+)`", enum_table))
    declared_types, declared_values = set(), set()
    for body, name in re.findall(r"typedef\s+enum\s*\{([^{}]*)\}\s*(Mv\w+)\s*;", header, re.DOTALL):
        if name in ENUMS:
            declared_types.add(name)
            declared_values.update(re.findall(r"\bMV_(?:KEY|MOUSE|PAD)_[A-Z0-9_]+\b", body))
    return (declared - mapped, mapped - declared, declared_types - documented_types,
            documented_types - declared_types, declared_values - documented_values,
            documented_values - declared_values)


def main():
    header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
    inventory = (ROOT / "docs/api/input-inventory.md").read_text(encoding="utf-8")
    errors = compare(header, inventory)
    labels = ("missing functions", "extra functions", "missing types", "extra types",
              "missing constants", "extra constants")
    for label, values in zip(labels, errors):
        if values:
            print(f"input: {label}: {', '.join(sorted(values))}")
    return int(any(errors))


if __name__ == "__main__":
    raise SystemExit(main())
