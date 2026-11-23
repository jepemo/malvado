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
local mv = require("malvado")
mv.import_globals()

local f = font {size = 50}
local title = write {
  font = f,
  text = "Hello, Malvado!",
  x = 400,
  y = 300,
  align = "center",
  color = WHITE,
}
change_text(title, {color = {255, 100, 100, 180}})
local width, height = text_size(f, "Hello")
draw_text {text = "x", x = 1, y = 2, color = {r = RED.r, g = RED[2], b = RED.b}}
delete_text(title)
set_color {10, 20, 30}
clear_screen(BLACK)
fade_to {color = TRANSPARENT, ticks = 30}
local view = camera_create {}
local world = world_create()
world_add_rect(world, {x = 10, y = 10, width = 20, height = 20})
local library = mv.fpg.parse("bytes")
local manifest = mv.fpg.parse_manifest(fpg_json(library))
local graphic = animation_sample(animation(library, {7, 42}), 50)
local index = fpg_id_at(library, 1)
local manifest_count_value = manifest_count(manifest)
local Player = process(function(self)
  self.fps = 30
  self.camera = view
  self.world = world
  self.region = region {x = 0, y = 0, width = 64, height = 48}
  self.scroll = scroll(5, 0)
  camera_follow(self)
  camera_follow(view, self)
  local x, y = camera_position()
  camera_set(x, y)
  camera_set(view, x, y)
  camera_move(1, 0)
  camera_move(view, x, y)
  world_move(1, 0)
  world_move(world, self, 1, 0)
  local message, explicit_message = recv(), recv(self)
  local elapsed, explicit_elapsed = animation_elapsed(), animation_elapsed(self)
  set_priority(10)
  set_priority(self, 0)
  if message then kill() end
  if explicit_message then assert(kill(self)) end
  self.graph = circle {radius = 20, color = GREEN}
  draw_rect {x = 0, y = 0, width = 4, height = 6, filled = false}
  draw_circle {x = 20, y = 20, radius = 5, color = BLUE}
  draw_line {x1 = 0, y1 = 0, x2 = 4, y2 = 6, thickness = 1.5}
  local image = load_image("sheet.png")
  local atlas = load_atlas(image, "{}")
  draw_image {image = image, x = 0.5, y = 0, pivot = "center"}
  draw_sprite {atlas = atlas, id = 42, x = 10, y = 10, angle = 90}
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
mv.change_text(id, {text = "y", color = {r = 1, g = 2, b = 3, a = 120}})
mv.set_color(mv.colors.RED)
mv.circle {radius = 2, color = {r = 2, g = 0, b = 0}}
mv.graphics.rect {width = 2, height = 2, color = mv.colors.PINK, pivot = "top-left"}
mv.draw_line {x1 = 0, y1 = 0, x2 = 2, y2 = 2, color = mv.colors.YELLOW}
local image = mv.load_image("sheet.png")
local atlas = mv.load_aseprite(image, "{}")
mv.draw_image {
  image = image, x = 10, y = 10, source = {x = 1, y = 2, width = 3, height = 4},
  pivot = {x = 1.5, y = 2}, scale = 2, angle = 90,
}
mv.draw_sprite {atlas = atlas, id = 42, x = 10.5, y = 10}
local clip = mv.graphics.region {x = 0, y = 0, width = 64, height = 48}
local view = mv.camera_create {viewport = clip}
local x, y = mv.camera_position(view)
mv.camera_move(view, x, y)
mv.camera_move(1, 0)
mv.camera_set(0, 0)
local world = mv.world_create()
local shape = mv.collision.circle(5)
local hit = mv.world_sweep(world, shape, nil, 1, 0)
local file = mv.fpg.parse("bytes")
local manifest = mv.fpg.parse_manifest(mv.fpg_json(file))
local opaque = mv.manifest_transparent_zero(manifest)
local actor = mv.process(function(self)
  self.file = atlas
  mv.camera_follow(self)
  mv.camera_set(view, 20, 40)
  mv.world_collision(world, self, {x = 2})
  local message, explicit_message = mv.recv(), mv.recv(self)
  local elapsed, explicit_elapsed = mv.animation_elapsed(), mv.animation_elapsed(self)
  local graphics_elapsed, graphics_explicit = mv.graphics.animation_elapsed(), mv.graphics.animation_elapsed(self)
  mv.set_priority(10)
  mv.set_priority(self, 0)
  mv.process.set_priority(10)
  mv.process.set_priority(self, 0)
  if message then mv.kill() end
  if explicit_message then assert(mv.kill(self)) end
  if elapsed then mv.process.kill() end
  if explicit_elapsed then assert(mv.process.kill(self)) end
  self.graph = mv.animation_sample(mv.animation(atlas, {1, 2}), 50)
  self.graph = mv.graphics.graph(42)
  self.graph = mv.graphics.animation(atlas, {1, 2}, {fps = 8})
end)
actor {}
mv.camera_destroy(view)
local current = mv.fade_color()
mv.set_color({current[1], current[2], current[3], current[4]})
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


def diagnose_project(project, prefix, failures):
    name = "positional-constructors.lua"
    constructors = [(f"{prefix}camera_create", "0, 0"), (f"{prefix}region", "0, 0, 1, 1")]
    if prefix:
        constructors.append(("mv.graphics.region", "0, 0, 1, 1"))
    (project / name).write_text(
        'local mv = require("malvado")\nmv.import_globals()\n' +
        "".join(f"{function}({arguments})\n" for function, arguments in constructors),
        encoding="utf-8")
    output = run_luals(project)
    diagnostics = diagnose(output, name)
    for line, (function, _) in enumerate(constructors, 3):
        if not any(number == line and message.startswith("Cannot assign `integer`")
                   for number, message in diagnostics):
            failures.append(f"{project.name}: positional {function} was not reported")
    return output


def main():
    if not shutil.which("lua-language-server"):
        print("lua-language-server is not on PATH", file=sys.stderr)
        return 1
    failures = []
    palette_fields = set(re.findall(
        r"^---@field (\w+) MvPaletteColor ", (TYPES / "lib/colors.d.lua").read_text(), re.MULTILINE))
    global_colors = set(re.findall(
        r"^---@type MvPaletteColor\n(\w+) =", (TYPES / "globals/malvado-globals.d.lua").read_text(), re.MULTILINE))
    documented_colors = set(re.findall(
        r"`mv\.colors\.(\w+)`", (ROOT / "docs/api/lua-globals.md").read_text()))
    if not palette_fields or palette_fields != global_colors or palette_fields != documented_colors:
        failures.append("palette fields, globals declarations, and global exports differ")
    public_functions = {
        name for name in re.findall(r"^\| `([a-z_]+)` \| `mv\.\1`",
                                   (ROOT / "docs/api/lua-globals.md").read_text(), re.MULTILINE)
        if name not in {"process", "collision"}
    }
    for definitions, prefix in [("lib/malvado.d.lua", r"malvado\."), ("globals/malvado-globals.d.lua", "")]:
        declared = set(re.findall(rf"^function {prefix}(\w+)\(", (TYPES / definitions).read_text(), re.MULTILINE))
        missing = sorted(public_functions - declared)
        if missing:
            failures.append(f"{definitions} has no declarations for {', '.join(missing)}")
    with tempfile.TemporaryDirectory(prefix="malvado-luals-") as temporary:
        # Project with globals enabled: the import_globals style must be clean
        # except for the deliberate invalid calls at the end.
        project = Path(temporary) / "globals-project"
        project.mkdir()
        write_luarc(project, with_globals=True)
        (project / "probe.lua").write_text(GLOBAL_PROBE, encoding="utf-8")
        output = diagnose_project(project, "", failures)
        messages = diagnose(output, "probe.lua")
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
        (project / "probe-globals.lua").write_text("write {}\nprint(RED)\n", encoding="utf-8")
        (project / "bad-drawing.lua").write_text(
            'local mv = require("malvado")\n'
            'local image = mv.load_image("sheet.png")\n'
            'mv.draw_image {image = image, x = 0, y = 0, scale = "large"}\n',
            encoding="utf-8")
        (project / "removed-methods.lua").write_text(
            'local mv = require("malvado")\n'
            'local view = mv.camera_create {}\n'
            'view:move(1, 2)\n'
            'local world = mv.world_create()\n'
            'world:add_rect({x = 0, y = 0, width = 1, height = 1})\n'
            'local file = mv.fpg.parse("bytes")\n'
            'file:count()\n',
            encoding="utf-8")
        output = diagnose_project(project, "mv.", failures)
        obsolete = diagnose(output, "removed-methods.lua")
        for line, method in [(3, "move"), (5, "add_rect"), (7, "count")]:
            if not any(number == line and f"Undefined field `{method}`" in message
                       for number, message in obsolete):
                failures.append(f"namespaced project: obsolete method {method} was not reported")
        if not any(line == 3 and "Cannot assign `string`" in message
                   for line, message in diagnose(output, "bad-drawing.lua")):
            failures.append("namespaced project: draw_image should reject a string scale")
        if "probe-globals.lua:1:1 [Warning] Undefined global `write`" not in output:
            failures.append("namespaced project: global `write` should be undefined")
        if "Undefined global `RED`" not in output:
            failures.append("namespaced project: global `RED` should be undefined")
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
