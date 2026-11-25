# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import zlib


runner, script, fixture, cli = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="malvado-fpg-") as temp:
    root = Path(temp)
    for transparent in (False, True):
        directory = root / ("transparent" if transparent else "opaque")
        directory.mkdir()
        env = dict(os.environ, MALVADO_FPG_TEST_DIR=str(directory),
                   MALVADO_FPG_TEST_TRANSPARENT=str(int(transparent)))
        subprocess.run([runner, "--headless", script], env=env, check=True)
        native = directory.with_name(directory.name + "-native")
        flags = ["--transparent-zero"] if transparent else []
        subprocess.run([cli, "fpg-export", *flags, fixture, str(native)], check=True)
        for name in ("manifest.json", "7.png", "42.png"):
            assert (directory / name).read_bytes() == (native / name).read_bytes()

    directory = root / "transparent"
    manifest = json.loads((directory / "manifest.json").read_text())
    source = Path(fixture).read_bytes()
    assert manifest["transparent_zero"] is True
    assert bytes.fromhex(manifest["palette_hex"]) == source[8:776]
    assert bytes.fromhex(manifest["gamma_hex"]) == source[776:1352]
    assert [item["id"] for item in manifest["sprites"]] == [7, 42]
    assert sorted(p.name for p in directory.iterdir()) == ["42.png", "7.png", "manifest.json"]
    expected = {7: (2, 2, bytes([0, 1, 1, 0])), 42: (2, 1, bytes([2, 1]))}
    for sprite in manifest["sprites"]:
        width, height, pixels = expected[sprite["id"]]
        assert (sprite["width"], sprite["height"]) == (width, height)
        data = (directory / sprite["image"]).read_bytes()
        assert data[:8] == b"\x89PNG\r\n\x1a\n"
        position, chunks = 8, {}
        while position < len(data):
            size = struct.unpack_from(">I", data, position)[0]
            kind = data[position + 4:position + 8]
            payload = data[position + 8:position + 8 + size]
            assert zlib.crc32(kind + payload) == struct.unpack_from(">I", data, position + 8 + size)[0]
            chunks[kind] = payload
            position += size + 12
        assert position == len(data) and chunks[b"tRNS"] == b"\0"
        assert chunks[b"PLTE"] == bytes(channel << 2 for channel in source[8:776])
        assert zlib.decompress(chunks[b"IDAT"]) == b"".join(
            b"\0" + pixels[y * width:(y + 1) * width] for y in range(height)
        )
    first, second = manifest["sprites"]
    assert bytes.fromhex(first["name_hex"]) == source[1360:1392]
    assert bytes.fromhex(first["filename_hex"]) == source[1392:1404]
    assert bytes.fromhex(second["name_hex"]) == source[1436:1468]
    assert bytes.fromhex(second["filename_hex"]) == source[1468:1480]
    assert first["points"] == [{"x": 1, "y": 0, "defined": True},
                               {"x": -1, "y": -1, "defined": False}]
    assert second["points"] == [{"x": 0, "y": 0, "defined": True}]
