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


def main():
    functions, globals_list = runtime_lists()
    module = module_definitions()
    global_defs = global_definitions()
    missing_module = sorted(set(functions) - module)
    missing_globals = sorted(set(globals_list) - global_defs)
    missing_globals_in_module = sorted(set(globals_list) - module)
    failures = []
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
