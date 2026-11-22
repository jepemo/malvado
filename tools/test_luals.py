#!/usr/bin/env python3
"""Validate the LuaLS type definitions with a real lua-language-server.

Creates temporary projects that use the definitions, runs the language
server's diagnosis, and verifies that valid code is clean and invalid code is
reported. The check requires `lua-language-server` on PATH; it does not run
any Malvado game code.
"""

from pathlib import Path
import json
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
TYPES = ROOT / "types"
GLOBAL_PROBE = """
require("malvado").import_globals()

local f = font {size = 50}
local title = write {
  font = f,
  text = "Hello, Malvado!",
  x = 400,
  y = 300,
  align = "center",
  color = {255, 255, 255, 255},
}
change_text(title, {color = {255, 100, 100, 180}})
local width, height = text_size(f, "Hello")
draw_text {text = "x", x = 1, y = 2}
delete_text(title)
set_color {10, 20, 30, 255}
clear_screen {25, 30, 45, 255}
fade_to {color = {0, 0, 0, 255}, ticks = 30}
local Player = process(function(self)
  self.fps = 30
  self.custom = 1
  local v = self.custom
  while true do frame() end
end)
Player {}

local a = write {font = f, text = "x", x = 1, y = 2, align = "bogus"}
local b = write {font = f, text = "x", x = 1, y = 2, color = {1, 2, 3}}
local c = missing_global_function()
"""
NAMESPACED_PROBE = """
local mv = require("malvado")
local f = mv.font {size = 50}
local id = mv.write {font = f, text = "x", x = 1, y = 2}
mv.change_text(id, {text = "y"})
local w, h = mv.text_size(id)
"""

def run_luals(directory):
    result = subprocess.run(
        ["lua-language-server", "--check=.", "--checklevel=Warning",
         "--check_format=pretty"],
        cwd=directory, capture_output=True, text=True)
    output = (result.stdout + result.stderr)
    return re.sub(r"\x1b\[[0-9;]*m", "", output)


def write_luarc(directory, with_globals):
    library = [str(TYPES / "lib")]
    if with_globals:
        library.append(str(TYPES / "globals"))
    (directory / ".luarc.json").write_text(json.dumps({
        "runtime.version": "Lua 5.4",
        "workspace.library": library,
        "workspace.checkThirdParty": False,
    }, indent=2) + "\n", encoding="utf-8")


def diagnose(output, name):
    """Return (line, message) pairs reported for one probe file."""
    found = []
    for match in re.finditer(rf"^{re.escape(name)}:(\d+):\d+ \[Warning\] ([^\n]+)", output, re.MULTILINE):
        found.append((int(match.group(1)), match.group(2)))
    return found


def main():
    if not shutil.which("lua-language-server"):
        print("lua-language-server is not on PATH", file=sys.stderr)
        return 1
    failures = []
    with tempfile.TemporaryDirectory(prefix="malvado-luals-") as temporary:
        # Project with globals enabled: the import_globals style must be clean
        # except for the deliberate invalid calls at the end.
        project = Path(temporary) / "globals-project"
        project.mkdir()
        write_luarc(project, with_globals=True)
        (project / "probe.lua").write_text(GLOBAL_PROBE, encoding="utf-8")
        output = run_luals(project)
        messages = diagnose(output, "probe.lua")
        deliberate = [item for item in messages if item[1].startswith("Cannot assign `string`")
                      or "Undefined global `missing_global_function`" in item[1]]
        if not any(item[1].startswith("Cannot assign `string`") for item in messages):
            failures.append("globals project: align = \"bogus\" was not reported")
        if not any("missing_global_function" in item[1] for item in messages):
            failures.append("globals project: the unknown global was not reported")
        for line, message in messages:
            if message.startswith("Cannot assign `string`") or "missing_global_function" in message:
                continue
            failures.append(f"globals project: unexpected diagnostic on line {line}: {message}")

        # Namespaced project without the globals definitions: the global style
        # must stay undefined, proving the globals file is opt-in.
        project = Path(temporary) / "namespaced-project"
        project.mkdir()
        write_luarc(project, with_globals=False)
        (project / "probe.lua").write_text(NAMESPACED_PROBE, encoding="utf-8")
        (project / "probe-globals.lua").write_text("write {}\n", encoding="utf-8")
        output = run_luals(project)
        if "probe-globals.lua:1:1 [Warning] Undefined global `write`" not in output:
            failures.append("namespaced project: global `write` should be undefined")
        if "probe.lua" in output and "[Warning]" in output:
            for line in output.splitlines():
                if line.startswith("probe.lua:"):
                    failures.append(f"namespaced project: unexpected diagnostic: {line.strip()}")
    for failure in failures:
        print(f"luals: {failure}", file=sys.stderr)
    if failures:
        return 1
    print("PASS: LuaLS definitions validate with a real language server")
    return 0


if __name__ == "__main__":
    sys.exit(main())
