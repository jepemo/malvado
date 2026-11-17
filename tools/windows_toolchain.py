#!/usr/bin/env python3
"""Select and verify the fixed Windows authoring toolchain before platform tests."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
import os
from pathlib import Path
import subprocess
import tempfile

GENERATOR = "Visual Studio 17 2022"
VS_BUILD = "17.14.37906.3"
MSVC = "14.44.35207"
COMPILER = "19.44.35230.0"
SDK = "10.0.26100.0"
PLATFORM = "x64,version=" + SDK
TOOLSET = "v143,host=x64,version=" + MSVC
ROOT = Path(__file__).resolve().parents[1]


def select_instance(instances):
    for item in instances:
        if item.get("installationVersion") != VS_BUILD or not item.get("installationPath"):
            continue
        instance = Path(item["installationPath"])
        if (instance / f"VC/Tools/MSVC/{MSVC}/bin/Hostx64/x64/cl.exe").is_file() and (instance / "MSBuild/Current/Bin/MSBuild.exe").is_file():
            return instance
    raise RuntimeError(f"Install VS 2022 build {VS_BUILD} with MSVC {MSVC}; no matching instance was found")


def selection_environment(instance, base):
    return dict(base, CMAKE_GENERATOR=GENERATOR,
                CMAKE_GENERATOR_INSTANCE=str(instance),
                CMAKE_GENERATOR_PLATFORM=PLATFORM, CMAKE_GENERATOR_TOOLSET=TOOLSET)


def validate_selection(facts, instance):
    expected = dict(generator=GENERATOR, platform=PLATFORM, toolset=TOOLSET,
                    vs_build=VS_BUILD, compiler_id="MSVC", compiler_version=COMPILER, sdk=SDK)
    for name, value in expected.items():
        if facts.get(name) != value:
            raise RuntimeError(f"Windows toolchain mismatch: {name}: expected {value!r}, got {facts.get(name)!r}")
    # CMake reports forward slashes; compare Windows paths without case changes
    # affecting selection. The compiler must belong to the selected instance.
    expected_compiler = instance / f"VC/Tools/MSVC/{MSVC}/bin/Hostx64/x64/cl.exe"
    for name, path in (("instance", instance), ("compiler", expected_compiler)):
        if facts.get(name, "").replace("\\", "/").casefold() != str(path).replace("\\", "/").casefold():
            raise RuntimeError(f"Windows toolchain mismatch: {name} is outside the selected installation")


def validate_cache(build, environment):
    cache = {}
    for line in (build / "CMakeCache.txt").read_text(encoding="utf-8").splitlines():
        if line and not line.startswith(("#", "//")) and "=" in line:
            name, value = line.split("=", 1)
            cache[name.split(":", 1)[0]] = value
    for name in ("CMAKE_GENERATOR", "CMAKE_GENERATOR_INSTANCE", "CMAKE_GENERATOR_PLATFORM", "CMAKE_GENERATOR_TOOLSET"):
        if cache.get(name, "").replace("\\", "/").casefold() != environment[name].replace("\\", "/").casefold():
            raise RuntimeError(f"Existing build has a different {name}; use a new --work directory: {build}")


def prepare(work):
    if os.name != "nt":
        raise RuntimeError("Windows toolchain verification requires native Windows")
    (work / "windows-toolchain.json").unlink(missing_ok=True)
    environment = dict(os.environ)
    programs_path = environment.get("PROGRAMFILES(X86)") or environment.get("ProgramFiles(x86)")
    if not programs_path:
        raise RuntimeError("Windows ProgramFiles(x86) is unavailable; cannot locate Visual Studio Installer")
    programs = Path(programs_path)
    vswhere = programs / "Microsoft Visual Studio/Installer/vswhere.exe"
    with (work / "windows-toolchain.log").open("w", encoding="utf-8") as log:
        discovery = subprocess.run([str(vswhere), "-products", "*", "-version", "[17.0,18.0)",
                                    "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-format", "json", "-utf8"],
                                   text=True, encoding="utf-8-sig", capture_output=True, check=True, timeout=30)
        log.write(discovery.stdout)
        log.flush()
        instance = select_instance(json.loads(discovery.stdout))
        kits = programs / "Windows Kits/10"
        for name in (f"Include/{SDK}/um/Windows.h", f"Include/{SDK}/ucrt/stdlib.h", f"Lib/{SDK}/um/x64/kernel32.lib"):
            if not (kits / name).is_file():
                raise RuntimeError(f"Install Windows SDK {SDK}: missing {name}")
        environment = selection_environment(instance, environment)
        # Keep the native probe paths short for MSBuild and clean only our tree.
        with tempfile.TemporaryDirectory(prefix="mv-vs-") as temporary:
            build = Path(temporary)
            command = ["cmake", "-S", str(ROOT / "tests/windows-toolchain"), "-B", str(build)]
            log.write(f"\n>>> {command!r}\n")
            log.flush()
            subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
            facts = dict(line.split("=", 1) for line in (build / "selection.txt").read_text().splitlines())
            validate_selection(facts, instance)
            validate_cache(build, environment)
            for config in ("Debug", "Release"):
                subprocess.run(["cmake", "--build", str(build), "--config", config], env=environment,
                               stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
                program = (build / f"program-{config}.txt").read_text().strip()
                result = subprocess.run([program], env=environment, text=True, capture_output=True, check=True, timeout=15)
                log.write(f"\n{config}: {result.stdout}{result.stderr}")
                if result.stdout.strip() != "Windows toolchain probe passed":
                    raise RuntimeError("Windows toolchain probe did not confirm execution")
            (work / "windows-toolchain.json").write_text(json.dumps(facts, indent=2) + "\n", encoding="utf-8")
    return environment
