# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

import struct
import subprocess
import sys
import zlib


def check(mode):
    data = subprocess.check_output([sys.argv[1], sys.argv[2], mode])
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    chunks = []
    position = 8
    while position < len(data):
        size = struct.unpack_from(">I", data, position)[0]
        kind = data[position + 4:position + 8]
        payload = data[position + 8:position + 8 + size]
        crc = struct.unpack_from(">I", data, position + 8 + size)[0]
        assert zlib.crc32(kind + payload) == crc
        chunks.append((kind, payload))
        position += size + 12
    assert position == len(data)
    assert [kind for kind, _ in chunks] == (
        [b"IHDR", b"PLTE"] + ([] if mode == "opaque" else [b"tRNS"]) + [b"IDAT", b"IEND"]
    )
    values = dict(chunks)
    width, height, depth, color, compression, filtering, interlace = struct.unpack(">IIBBBBB", values[b"IHDR"])
    assert (depth, color, compression, filtering, interlace) == (8, 3, 0, 0, 0)
    assert len(values[b"PLTE"]) == 768 and values[b"PLTE"][3:6] == bytes([252, 0, 0])
    if mode != "opaque":
        assert values[b"tRNS"] == b"\0"
    raw = zlib.decompress(values[b"IDAT"])
    if mode == "large":
        assert (width, height) == (257, 257)
        expected = bytes(i % 256 for i in range(width * height))
    else:
        assert (width, height) == (2, 2)
        expected = bytes([0, 1, 1, 0])
    assert raw == b"".join(b"\0" + expected[y * width:(y + 1) * width] for y in range(height))
    assert values[b"IEND"] == b""


for case in ("opaque", "transparent", "large"):
    check(case)
