#!/usr/bin/env python3
"""Check that the LuaLS type definitions cover the public Lua module.

The Lua registration lists in bindings/lua/runtime.c are the source of truth.
This test verifies that every exported function and constant has a matching
declaration in the distributed type definitions.
"""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / "bindings/lua/runtime.c"
MODULE = ROOT / "types/lib/malvado.d.lua"
GLOBALS = ROOT / "types/globals/malvado-globals.d.lua"


def runtime_lists():
    source = RUNTIME.read_text(encoding="utf-8")
    module_match = re.search(
        r"static const luaL_Reg functions\[\] = \{(.*?)\{NULL, NULL\}\s*\};",
        source, re.DOTALL)
    global_match = re.search(
        r"static const char \*const global_names\[\] = \{(.*?)NULL\s*\};",
        source, re.DOTALL)
    if not module_match or not global_match:
        raise ValueError("Unexpected runtime.c layout")
    functions = re.findall(r'\{"(\w+)", \w+\}', module_match.group(1))
    globals_list = re.findall(r'"(\w+)"', global_match.group(1))
    return functions, globals_list


def module_definitions():
    source = MODULE.read_text(encoding="utf-8")
    functions = set(re.findall(r"^function malvado\.(\w+)\(", source, re.MULTILINE))
    fields = set(re.findall(r"^---@field (\w+)", source, re.MULTILINE))
    constants = set(re.findall(r"^malvado\.(\w+) =", source, re.MULTILINE))
    return functions | fields | constants


def global_definitions():
    source = GLOBALS.read_text(encoding="utf-8")
    functions = set(re.findall(r"^function (\w+)\(", source, re.MULTILINE))
    variables = set(re.findall(r"^(\w+) = ", source, re.MULTILINE))
    return functions | variables


# Match the receiver labels in the public method table to their LuaLS types.
RECEIVERS = {
    "camera": "MvCamera", "world": "MvCollisionWorld", "image": "MvImage",
    "animation": "MvAnimation", "handle": "MvProcess", "sound": "MvSound",
    "voice": "MvVoice", "music": "MvMusic", "atlas": "MvAtlas",
    "fpg": "MvFpg", "manifest": "MvFpgManifest",
}


def check_method_definitions():
    guide = (ROOT / "docs/api/lua-instance-methods.md").read_text(encoding="utf-8")
    public = set(re.findall(r"^\| `(\w+):(\w+)\(", guide, re.MULTILINE))
    names = {name: receiver for receiver, name in RECEIVERS.items()}
    definitions = set()
    for path in MODULE.parent.glob("*.d.lua"):
        source = path.read_text(encoding="utf-8")
        for method, cls in re.findall(r"^---@field (\w+) (?:\()?fun\(self: (\w+)", source, re.MULTILINE):
            if cls in names:
                definitions.add((names[cls], method))
        for cls, variable in re.findall(r"^---@class (\w+)\s+local (\w+) = \{\}", source, re.MULTILINE):
            if cls in names:
                methods = re.findall(rf"^function {variable}:(\w+)\(", source, re.MULTILINE)
                definitions.update((names[cls], method) for method in methods)
    for label, difference in (("missing", public - definitions), ("stale", definitions - public)):
        if difference:
            calls = ", ".join(f"{receiver}:{method}" for receiver, method in sorted(difference))
            yield f"{label} LuaLS instance method definitions: {calls}"


def main():
    functions, globals_list = runtime_lists()
    module = module_definitions()
    global_defs = global_definitions()
    missing_module = sorted(set(functions) - module)
    missing_globals = sorted(set(globals_list) - global_defs)
    missing_globals_in_module = sorted(set(globals_list) - module)
    failures = list(check_method_definitions())
    if missing_module:
        failures.append(f"module functions without type definitions: {', '.join(missing_module)}")
    if missing_globals:
        failures.append(f"globals without type definitions: {', '.join(missing_globals)}")
    if missing_globals_in_module:
        failures.append(f"module fields for globals without type definitions: {', '.join(missing_globals_in_module)}")
    for failure in failures:
        print(f"lua types: {failure}", file=sys.stderr)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
