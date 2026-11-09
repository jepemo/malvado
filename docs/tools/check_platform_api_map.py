#!/usr/bin/env python3
"""Check the platform, input, and audio inventories against platform.h."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
FUNCTION = re.compile(r"\b(mv_[a-z][a-z0-9_]*)\s*\(")
TYPE = re.compile(r"\}\s*(Mv\w+)\s*;|typedef\s+struct\s+\w+\s+(Mv\w+)\s*;")


def table(document, heading):
    return document.split(heading, 1)[1].split("\n\n", 1)[0]


def compare(header, platform, inputs, audio):
    header = COMMENT.sub("", header)
    functions = set(FUNCTION.findall(header))
    types = {a or b for a, b in TYPE.findall(header)}
    maps = [table(platform, "| Area | C | Lua | Evidence |"),
            table(inputs, "| Area | C | Lua | Examples and automated evidence |"),
            table(audio, "| Area | C | Lua | Examples and automated evidence |")]
    type_maps = [table(platform, "| C types | Meaning and Lua relationship |"),
                 table(inputs, "| C type | C constants | Lua relationship |"),
                 table(audio, "| C type | Lua relationship and lifetime |")]
    mapped_functions = set(re.findall(r"`(mv_[a-z][a-z0-9_]*)`", "\n".join(maps)))
    mapped_types = set(re.findall(r"`(Mv\w+)`", "\n".join(type_maps)))
    return functions - mapped_functions, mapped_functions - functions, types - mapped_types, mapped_types - types


def main():
    header = (ROOT / "src/include/malvado/platform.h").read_text(encoding="utf-8")
    platform, inputs, audio = [(ROOT / "docs/api" / name).read_text(encoding="utf-8")
                               for name in ("platform-inventory.md", "input-inventory.md", "audio-inventory.md")]
    errors = compare(header, platform, inputs, audio)
    for label, names in zip(("missing functions", "extra functions", "missing types", "extra types"), errors):
        if names:
            print(f"platform: {label}: {', '.join(sorted(names))}")
    return int(any(errors))


if __name__ == "__main__":
    raise SystemExit(main())
