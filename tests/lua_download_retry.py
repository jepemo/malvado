#!/usr/bin/env python3
"""Exercise the real Lua FetchContent declaration against a local HTTP origin."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
from http.server import BaseHTTPRequestHandler, HTTPServer
import io
import os
from pathlib import Path
import re
import subprocess
import tarfile
import tempfile
import threading

ROOT = Path(__file__).resolve().parents[1]


def archive(payload=b"Pinned test payload\n"):
    output = io.BytesIO()
    with tarfile.open(fileobj=output, mode="w:gz") as tar:
        info = tarfile.TarInfo("lua/README")
        info.size = len(payload)
        tar.addfile(info, io.BytesIO(payload))
    return output.getvalue()


def scenario(mode, declaration, payload):
    corrupt_payload = archive(b"Incorrect release payload\n")
    class Handler(BaseHTTPRequestHandler):
        calls = 0

        def do_GET(self):
            Handler.calls += 1
            failed = mode == "permanent" or (mode == "transient" and Handler.calls == 1)
            body = corrupt_payload if mode == "corrupt" else payload
            self.send_response(500 if failed else 200)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, *args):
            pass

    server = HTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        with tempfile.TemporaryDirectory(prefix="malvado-lua-download-") as directory:
            root = Path(directory)
            url = f"http://127.0.0.1:{server.server_port}/lua.tar.gz"
            local = declaration.replace("https://www.lua.org/ftp/lua-5.4.8.tar.gz", url + "?origin=www")
            local = local.replace("https://lua.org/ftp/lua-5.4.8.tar.gz", url + "?origin=bare")
            local = local.replace(
                "4f18ddae154e793e46eeab727c59ef1c0c0c2b744e7b94219710d76f530629ae",
                hashlib.sha256(payload).hexdigest())
            (root / "CMakeLists.txt").write_text(
                'cmake_minimum_required(VERSION 3.20)\nproject(download NONE)\n'
                'include(FetchContent)\n' + local + '\nFetchContent_MakeAvailable(lua)\n')
            result = subprocess.run(["cmake", "-S", str(root), "-B", str(root / "build")],
                                    capture_output=True, text=True, timeout=40,
                                    env=dict(os.environ, NO_PROXY="127.0.0.1,localhost",
                                             no_proxy="127.0.0.1,localhost"))
            extracted = root / "build/_deps/lua-src/README"
            if mode == "transient":
                assert result.returncode == 0, result.stdout + result.stderr
                assert Handler.calls == 2 and extracted.read_bytes() == b"Pinned test payload\n"
            else:
                assert result.returncode != 0, "failed/corrupt origin was accepted"
                assert Handler.calls >= 2 and not extracted.exists(), result.stdout + result.stderr
            print(f"PASS: {mode}, {Handler.calls} requests")
    finally:
        server.shutdown()
        thread.join()
        server.server_close()


def main():
    source = (ROOT / "CMakeLists.txt").read_text()
    declaration = re.search(r"FetchContent_Declare\(lua\s+URL\s.*?\)", source, re.S).group()
    assert declaration.count("https://www.lua.org/ftp/lua-5.4.8.tar.gz") == 1
    assert declaration.count("https://lua.org/ftp/lua-5.4.8.tar.gz") == 1
    assert "SHA256=4f18ddae154e793e46eeab727c59ef1c0c0c2b744e7b94219710d76f530629ae" in declaration
    payload = archive()
    for mode in ("transient", "permanent", "corrupt"):
        scenario(mode, declaration, payload)


if __name__ == "__main__":
    main()
