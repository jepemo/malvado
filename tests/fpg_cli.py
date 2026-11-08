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

cli, source = sys.argv[1:]


def run(*args, **kwargs):
    return subprocess.run([cli, *map(str, args)], stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, **kwargs)


with tempfile.TemporaryDirectory(prefix="malvado-fpg-cli-") as temp:
    root = Path(temp)
    source_data = Path(source).read_bytes()
    for transparent in (False, True):
        output = root / ("transparent" if transparent else "opaque")
        options = ["--transparent-zero"] if transparent else []
        assert run("fpg-export", *options, source, output).returncode == 0
        manifest = json.loads((output / "manifest.json").read_text())
        assert manifest["transparent_zero"] == transparent
        assert bytes.fromhex(manifest["palette_hex"]) == source_data[8:776]
        assert [item["id"] for item in manifest["sprites"]] == [7, 42]
        assert sorted(p.name for p in output.iterdir()) == ["42.png", "7.png", "manifest.json"]
        data = (output / "7.png").read_bytes()
        pos, chunks = 8, {}
        assert data[:8] == b"\x89PNG\r\n\x1a\n"
        while pos < len(data):
            size = struct.unpack_from(">I", data, pos)[0]
            kind, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + size]
            assert zlib.crc32(kind + body) == struct.unpack_from(">I", data, pos + 8 + size)[0]
            chunks[kind] = body
            pos += size + 12
        assert (b"tRNS" in chunks) == transparent
        assert zlib.decompress(chunks[b"IDAT"]) == b"\0\0\1\0\1\0"
        assert run("fpg-export", source, output).returncode != 0
        assert (output / "7.png").read_bytes() == data
    bad = root / "bad.fpg"
    bad.write_bytes(b"invalid")
    assert run("fpg-export", bad, root / "invalid-output").returncode != 0
    assert not (root / "invalid-output").exists()
    assert run("fpg-export", root / "missing", root / "missing-output").returncode != 0
    assert run("fpg-export", "--unknown", source, root / "output").returncode == 2
    assert run("fpg-export", source).returncode == 2

    if os.name == "posix":
        import resource
        import signal

        def limit_file():
            signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
            resource.setrlimit(resource.RLIMIT_FSIZE, (100, 100))

        incomplete = root / "incomplete"
        assert run("fpg-export", source, incomplete, preexec_fn=limit_file).returncode != 0
        assert not incomplete.exists()
