#!/usr/bin/env python3
"""Check repository file links and Markdown anchors in current alpha.7 guides."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from functools import lru_cache
import html
from pathlib import Path
import re
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
LINK = re.compile(r'\]\((<[^>]+>|[^\s)]+)(?:\s+"[^"]*")?\)|^\s*\[[^\]]+\]:\s*(<[^>]+>|[^\s]+)', re.MULTILINE)


def prose(text):
    """Keep line positions, but do not interpret links inside fenced examples."""
    lines = []
    fence = None
    for line in text.splitlines(keepends=True):
        match = re.match(r"^\s{0,3}(`{3,}|~{3,})", line)
        if fence:
            if match and match[1][0] == fence[0] and len(match[1]) >= len(fence):
                fence = None
            lines.append("\n" if line.endswith("\n") else "")
        elif match:
            fence = match[1]
            lines.append("\n" if line.endswith("\n") else "")
        else:
            lines.append(line)
    return "".join(lines)


@lru_cache(maxsize=None)
def anchors(path):
    text = prose(path.read_text())
    result = set(re.findall(r'<[^>]+\b(?:id|name)=[\'\"]([^\'\"]+)', text))
    seen = {}
    for heading in re.findall(r"^#{1,6}\s+(.+?)\s*#*$", text, re.MULTILINE):
        heading = re.sub(r"\[([^]]+)\]\([^)]+\)", r"\1", heading)
        heading = html.unescape(re.sub(r"<[^>]+>", "", heading)).lower()
        slug = re.sub(r"[^\w\- ]", "", heading).replace(" ", "-")
        index = seen.get(slug, 0)
        seen[slug] = index + 1
        result.add(slug + (f"-{index}" if index else ""))
    return result


def check_documents(paths, root):
    errors = []
    checked = 0
    anchors.cache_clear()
    github = "https://github.com/jepemo/malvado/blob/master/"
    for path in paths:
        text = prose(path.read_text())
        for match in LINK.finditer(text):
            link = (match[1] or match[2]).strip("<>")
            if link.startswith(github):
                link = "/" + link.removeprefix(github)
            target, _, fragment = link.partition("#")
            if ":" in target or target.startswith("//"):
                continue
            destination = (root / unquote(target).lstrip("/") if target.startswith("/") else
                           path.parent / unquote(target) if target else path).resolve()
            line = text.count("\n", 0, match.start()) + 1
            checked += 1
            if not destination.exists():
                errors.append(f"{path}:{line}: missing link target: {link}")
            elif fragment and destination.suffix == ".md" and unquote(fragment) not in anchors(destination):
                errors.append(f"{path}:{line}: missing Markdown anchor: {link}")
    return checked, errors


def documents():
    paths = [ROOT / "README.md", ROOT / "bindings/rust/README.md",
             ROOT / "docs/specs/cli.md", ROOT / "docs/specs/rust-processes.md",
             ROOT / "docs/versions/README.md"]
    for directory in ("api", "site", "development", "architecture"):
        paths.extend((ROOT / "docs" / directory).glob("*.md"))
    paths.extend((ROOT / "docs/versions").glob("2.0.0-alpha.7*.md"))
    return sorted(set(paths))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", type=Path)
    args = parser.parse_args()
    paths = args.paths or documents()
    checked, errors = check_documents(paths, ROOT)
    if errors:
        raise SystemExit("\n".join(errors))
    print(f"PASS: {checked} file/anchor links in {len(paths)} current guide documents")


if __name__ == "__main__":
    main()
