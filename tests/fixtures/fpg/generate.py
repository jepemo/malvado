# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0
"""Generate an original BennuGD 8-bit FPG fixture."""

from pathlib import Path
import struct

palette = bytearray(256 * 3)
palette[3:6] = bytes((63, 0, 0))
palette[6:9] = bytes((0, 63, 0))


def sprite(code, name, width, height, points, pixels):
    header = struct.pack("<ii32s12siii", code, 0, name.encode("ascii"), b"", width, height, len(points))
    controls = b"".join(struct.pack("<hh", x, y) for x, y in points)
    assert len(pixels) == width * height
    return header + controls + bytes(pixels)


data = b"fpg\x1a\r\n\x00\x00" + palette + bytes(576)
data += sprite(7, "red", 2, 2, [(1, 0), (-1, -1)], [0, 1, 1, 0])
data += sprite(42, "green", 2, 1, [(0, 0)], [2, 1])
Path(__file__).with_name("bennugd-8.fpg").write_bytes(data)
