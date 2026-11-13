#!/usr/bin/env python3
"""Compile and run the actual Rust API guide blocks against an installed SDK."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def snippets():
    for path in sorted((ROOT / "docs/api").glob("*.md")):
        for index, match in enumerate(re.finditer(r"^```(rust[^\n]*)\n(.*?)^```", path.read_text(),
                                                  re.MULTILINE | re.DOTALL), 1):
            if match[1] != "rust":
                raise ValueError(f"{path}: unsupported snippet mode {match[1]!r}; provide a runnable example")
            # Use Rustdoc's hidden-line convention without copying the example.
            body = "\n".join(line[2:] if line.startswith("# ") else line
                             for line in match[2].splitlines())
            yield path, index, body


def fixtures(directory):
    source = ROOT / "tests/fixtures"
    for name, original in {
        "ship.png": source / "red.png",
        "font.ttf": source / "fonts/AnonymousPro-Bold.ttf",
        "OFL.txt": source / "fonts/OFL.txt",
        "graphics.fpg": source / "fpg/bennugd-8.fpg",
        "jump.wav": source / "silence.wav",
        "theme.ogg": source / "silence.ogg",
    }.items():
        shutil.copyfile(original, directory / name)
    (directory / "characters.json").write_text(json.dumps({
        "schema": 1, "kind": "graphics-library", "image": "ship.png",
        "sprites": [{"id": 7, "name": "hero", "x": 0, "y": 0, "width": 4, "height": 8}],
    }))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--release", action="store_true")
    args = parser.parse_args()
    sdk = args.sdk.resolve(strict=True)
    crate = sdk / "share/malvado/rust/malvado"
    if not (crate / "Cargo.toml").is_file():
        raise SystemExit("SDK is missing installed Rust crate sources")
    target = ROOT / "build-rust-docs/target"
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(sdk), CARGO_TARGET_DIR=str(target),
               RUSTUP_AUTO_INSTALL="0")
    with tempfile.TemporaryDirectory(prefix="malvado-guide-") as temporary:
        work = Path(temporary)
        (work / "src/bin").mkdir(parents=True)
        (work / "Cargo.toml").write_text(
            '[package]\nname="malvado-guide-checks"\nversion="0.0.0"\nedition="2024"\n'
            '[dependencies]\nmalvado={path=' + json.dumps(str(crate)) + ',features=["graphics","assets"]}\n')
        cases = list(snippets())
        if not cases:
            raise RuntimeError("no Rust guide snippets found")
        for path, index, body in cases:
            name = path.stem.replace("-", "_") + f"_{index}"
            (work / f"src/bin/{name}.rs").write_text(
                f"// {path.relative_to(ROOT)}, Rust block {index}\n"
                "fn main() -> Result<(), Box<dyn std::error::Error>> {\n"
                "    (|| {\n" + body + "\n    })()?;\n    Ok(())\n}\n")
        fixtures(work)
        subprocess.run(["cargo", "build", "--offline", "--bins",
                        *(["--release"] if args.release else [])], cwd=work, env=env, check=True, timeout=180)
        profile = "release" if args.release else "debug"
        for path, index, _ in cases:
            name = path.stem.replace("-", "_") + f"_{index}"
            subprocess.run([str(target / profile / name)], cwd=work, env=env, check=True, timeout=15,
                           stdout=subprocess.DEVNULL)
        print(f"PASS: {len(cases)} actual Rust guide snippets compiled and ran ({profile})", flush=True)


if __name__ == "__main__":
    main()
