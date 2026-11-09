#!/usr/bin/env python3
"""Check documented audio API names against the public C header."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
FUNCTION = re.compile(r"\b(mv_[a-z][a-z0-9_]*)\s*\(")
PREFIXES = ("mv_audio_", "mv_sound_", "mv_music_")
TYPES = {"MvSound", "MvMusic"}


def compare(header, inventory):
    header = COMMENT.sub("", header)
    declared = {name for name in FUNCTION.findall(header) if name.startswith(PREFIXES)}
    table = inventory.split("| Area | C | Lua | Examples and automated evidence |", 1)[1].split("\n\n", 1)[0]
    mapped = set(re.findall(r"`(mv_[a-z][a-z0-9_]*)`", table))
    declared_types = set(re.findall(r"typedef\s+struct\s+\w+\s+(Mv\w+)\s*;", header)) & TYPES
    type_table = inventory.split("## C audio types", 1)[1].split("\n\n", 2)[1]
    mapped_types = set(re.findall(r"`(Mv\w+)`", type_table))
    return declared - mapped, mapped - declared, declared_types - mapped_types, mapped_types - declared_types


def main():
    header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
    inventory = (ROOT / "docs/api/audio-inventory.md").read_text(encoding="utf-8")
    errors = compare(header, inventory)
    for label, values in zip(("missing functions", "extra functions", "missing types", "extra types"), errors):
        if values:
            print(f"audio: {label}: {', '.join(sorted(values))}")
    return int(any(errors))


if __name__ == "__main__":
    raise SystemExit(main())
