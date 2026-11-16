#!/usr/bin/env python3
"""Verify the Android headless C/Lua foundation on an explicitly selected emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import uuid

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
NDK_VERSION = "27.2.12479018"
ANDROID_API = 28
EMULATOR_API = "34"


def run(log, args, *, timeout=600, cwd=ROOT, check=True):
    log.write(f"\n>>> {args!r}\n")
    log.flush()
    result = subprocess.run([str(arg) for arg in args], cwd=cwd, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=timeout)
    log.write(result.stdout)
    log.flush()
    if check and result.returncode:
        raise RuntimeError(f"Command exited {result.returncode}: {args!r}\n{result.stdout[-6000:]}")
    return result


def properties(path):
    return dict(tuple(part.strip() for part in line.split("=", 1)) for line in path.read_text().splitlines()
                if "=" in line and not line.startswith("#"))


def preflight(sdk, serial, log):
    ndk = sdk / "ndk" / NDK_VERSION
    if not ndk.is_dir() or properties(ndk / "source.properties").get("Pkg.Revision") != NDK_VERSION:
        raise RuntimeError(f"Install ndk;{NDK_VERSION} in {sdk}")
    adb = sdk / "platform-tools" / ("adb.exe" if os.name == "nt" else "adb")
    prefix = [adb, "-s", serial]
    run(log, prefix + ["get-state"], timeout=15)
    facts = {}
    for name in ("ro.kernel.qemu", "ro.product.cpu.abi", "ro.build.version.sdk", "sys.boot_completed"):
        facts[name] = run(log, prefix + ["shell", "getprop", name], timeout=15).stdout.strip()
    if facts["ro.kernel.qemu"] != "1":
        raise RuntimeError("Use an emulator, not a physical device, for this verification")
    if facts["sys.boot_completed"] != "1" or facts["ro.build.version.sdk"] != EMULATOR_API:
        raise RuntimeError(f"A booted Android API {EMULATOR_API} emulator is required")
    abi = facts["ro.product.cpu.abi"]
    if abi not in ("arm64-v8a", "x86_64"):
        raise RuntimeError(f"Unverified Android ABI: {abi}")
    facts.update(host=f"{platform.system()}-{platform.machine()}", ndk=NDK_VERSION,
                 native_api=ANDROID_API, serial=serial)
    for tool in ("cmake", "python3"):
        facts[tool] = run(log, [tool, "--version"], timeout=15).stdout.strip()
    for name in ("platform-tools", "emulator"):
        facts[name] = properties(sdk / name / "source.properties").get("Pkg.Revision")
    return ndk, prefix, abi, facts


def shell(prefix, log, arguments, *, cwd=None, environment=(), timeout=60, check=True):
    # adb shell uses a remote shell even when the host uses an argv list.
    # Quote every remote argument, including paths and environment assignments.
    command = shlex.join(["env", *environment, *map(str, arguments)])
    if cwd:
        command = "cd " + shlex.quote(cwd) + " || exit 126; " + command
    marker = "MALVADO_ANDROID_EXIT_" + uuid.uuid4().hex
    command += f'; result=$?; printf "\\n{marker}:%s\\n" "$result"; exit "$result"'
    result = run(log, prefix + ["shell", command], timeout=timeout, check=False)
    ending = f"\n{marker}:{result.returncode}\n"
    if not result.stdout.endswith(ending):
        raise RuntimeError(f"Android command transport failed: {result.stdout[-2000:]}")
    result.stdout = result.stdout[:-len(ending)]
    if check and result.returncode:
        raise RuntimeError(f"Android command exited {result.returncode}: {result.stdout[-6000:]}")
    return result


def remap(value, build, remote):
    for local, target in ((build, remote + "/bin"), (ROOT, remote + "/source")):
        if value == str(local):
            return target
        if value.startswith(str(local) + os.sep):
            return target + "/" + Path(value).relative_to(local).as_posix()
    return value


def native_tests(catalog, build):
    selected = []
    for test in catalog["tests"]:
        command = test.get("command", [])
        if not command:
            continue
        executable = Path(command[0])
        # Host CMake/Python/shell checks and the desktop CLI are not Android tests.
        if executable.parent != build or executable.name in ("malvado", "malvado-game", "malvado-native-game"):
            continue
        if not executable.is_file() or executable.read_bytes()[:4] != b"\x7fELF":
            raise RuntimeError(f"Missing Android executable: {executable}")
        selected.append(test)
    if not selected:
        raise RuntimeError("No Android executable tests were discovered")
    return selected


def check_result(test, result):
    settings = {item["name"]: item["value"] for item in test.get("properties", [])}
    failed = result.returncode != 0
    # WILL_FAIL accepts ordinary program errors, never signals or shell failures.
    if settings.get("WILL_FAIL"):
        valid = 0 < result.returncode < 126
    else:
        valid = not failed
    for expression in settings.get("PASS_REGULAR_EXPRESSION", []):
        valid = valid and re.search(expression, result.stdout) is not None
    for expression in settings.get("FAIL_REGULAR_EXPRESSION", []):
        valid = valid and re.search(expression, result.stdout) is None
    if not valid:
        raise RuntimeError(f"{test['name']}: exit {result.returncode}\n{result.stdout[-6000:]}")


def stage_sources(stage):
    names = subprocess.check_output(
        ["git", "ls-files", "-z", "tests", "examples", "tools/benchmarks", "docs/api/lua-globals.md", "docs/api/lua-namespaces.md"], cwd=ROOT).decode().split("\0")
    for name in filter(None, names):
        path = ROOT / name
        # Only committed test inputs. Never stage caches, local output or native source.
        if path.suffix in (".c", ".h", ".cpp", ".py", ".sh", ".cmake"):
            continue
        if path.is_symlink():
            raise RuntimeError(f"Test input cannot be a symlink: {name}")
        target = stage / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)
    return stage


def verify_installed_sdk(log, work, config, build, ndk, abi, prefix, remote):
    staged = build / "staged sdk"
    installed = build / "relocated sdk"
    for path in (staged, installed):
        if path.exists():
            shutil.rmtree(path)
    run(log, ["cmake", "--install", build, "--config", config, "--prefix", staged])
    staged.rename(installed)
    info = properties(installed / "share/malvado/malvado-sdk-info.txt")
    if info["system"] != "android":
        raise RuntimeError("Installed SDK does not identify its Android target")
    for name in ("LICENSE", "Lua-readme.html", "cJSON-LICENSE"):
        if not (installed / "share/licenses/malvado" / name).is_file():
            raise RuntimeError(f"Missing SDK notice: {name}")
    project = build / "external C game"
    project.mkdir(exist_ok=True)
    for name in ("CMakeLists.txt", "main.c"):
        shutil.copyfile(ROOT / "tests/android-sdk" / name, project / name)
    output = project / "build"
    run(log, ["cmake", "-S", project, "-B", output,
              f"-DCMAKE_TOOLCHAIN_FILE={ndk / 'build/cmake/android.toolchain.cmake'}",
              f"-DANDROID_ABI={abi}", f"-DANDROID_PLATFORM=android-{ANDROID_API}",
              f"-DCMAKE_BUILD_TYPE={config}", f"-DMALVADO_SDK_PREFIX={installed}",
              f"-DEXPECTED_VERSION={info['version']}"])
    run(log, ["cmake", "--build", output, "--parallel", "2"])
    # Only the consumer binary reaches this directory. No SDK or source tree is present.
    destination = remote + "/external game"
    shell(prefix, log, ["mkdir", "-p", destination])
    run(log, prefix + ["push", output / "android-sdk-consumer", destination + "/game"])
    result = shell(prefix, log, [destination + "/game"], cwd=destination)
    if "Android installed SDK: process, handle, asset and shutdown passed" not in result.stdout:
        raise RuntimeError("Installed SDK consumer did not produce its acceptance trace")
    print(f"PASS: Android {abi} {config}: external installed C SDK", flush=True)


def verify_lua_contracts(log, work, config, build, prefix, remote):
    if os.name == "nt":
        raise RuntimeError("Android contract verification currently requires a macOS/Linux authoring host")
    wrappers = work / config / "host contract wrappers"
    wrappers.mkdir(parents=True, exist_ok=True)

    def wrapper(program):
        run(log, prefix + ["push", build / program, remote + "/bin/" + program])
        state = wrappers / (program + ".json")
        state.write_text(json.dumps({
            "adb": [str(value) for value in prefix],
            "program": remote + "/bin/" + program,
            "cwd": remote + "/source",
            "paths": [[str(wrappers), remote + "/generated"],
                      [str(build), remote + "/bin"],
                      [str(ROOT), remote + "/source"]],
        }))
        launcher = wrappers / program
        launcher.write_text("#!/bin/sh\nexec " + shlex.join([
            sys.executable, str(ROOT / "tools/android_command.py"), "--state", str(state), "--"
        ]) + ' "$@"\n')
        launcher.chmod(0o755)
        return str(launcher)

    runner = wrapper("malvado-run")
    contracts = [
        ("invalid game configuration", "declarative_invalid.cmake", {}),
        ("startup results", "start_results.cmake", {}),
        ("startup failure cleanup", "start_failure_cleanup.cmake", {}),
        ("process error traceback", "check_error.cmake", {"SCRIPT": ROOT / "tests/error.lua"}),
        ("shutdown handles", "handle_shutdown.cmake", {"SCRIPT": ROOT / "tests/handle_shutdown.lua"}),
        ("close failure cleanup", "check_close_failure.cmake", {"SCRIPT": ROOT / "tests/close_failure.lua"}),
        ("natural close failure", "check_close_failure.cmake", {
            "SCRIPT": ROOT / "tests/close_failure_natural.lua", "EXPECTED_ERROR": "natural close failed"}),
        ("close after process error", "check_close_failure.cmake", {
            "SCRIPT": ROOT / "tests/close_failure_error.lua", "EXPECTED_ERROR": "close after error"}),
        ("close after self kill", "check_close_failure.cmake", {
            "SCRIPT": ROOT / "tests/close_failure_self.lua", "EXPECTED_ERROR": "close after self-kill"}),
    ]
    for name, script, values in contracts:
        run(log, ["cmake", f"-DPROGRAM={runner}",
                  *[f"-D{key}={value}" for key, value in values.items()],
                  "-P", ROOT / "tests" / script], cwd=wrappers, timeout=90)
        print(f"PASS: Android {config}: {name}", flush=True)
    traces = [
        ("bound-collider-test", "bound_collider.lua", "bound_collider.txt"),
        ("camera-follow-test", "alpha7_camera_follow.lua", "camera_follow.txt"),
        ("alpha7-world-test", "alpha7_world.lua", "world.txt"),
        ("alpha7-fade-test", "alpha7_fade.lua", "fade.txt"),
        ("alpha7-lifecycle-test", "alpha7_lifecycle.lua", "lifecycle.txt"),
        ("alpha7-baseline-test", "alpha7_baseline.lua", "alpha7_baseline.txt"),
    ]
    for program, script, expected in traces:
        run(log, ["cmake", f"-DC_PROGRAM={wrapper(program)}", f"-DLUA_PROGRAM={runner}",
                  f"-DLUA_SCRIPT={ROOT / 'tests' / script}",
                  f"-DEXPECTED_FILE={ROOT / 'bindings/rust/malvado/tests/fixtures' / expected}",
                  "-P", ROOT / "tests/compare_parity.cmake"], cwd=wrappers, timeout=45)
        print(f"PASS: Android {config}: shared C/Lua {expected}", flush=True)
    return len(contracts) + len(traces)


def verify_profile(work, config, ndk, prefix, abi):
    build = work / config
    with (work / f"{config}.log").open("w", encoding="utf-8") as log:
        run(log, ["cmake", "-S", ROOT, "-B", build,
                  f"-DCMAKE_TOOLCHAIN_FILE={ndk / 'build/cmake/android.toolchain.cmake'}",
                  f"-DANDROID_ABI={abi}", f"-DANDROID_PLATFORM=android-{ANDROID_API}",
                  f"-DCMAKE_BUILD_TYPE={config}", "-DCMAKE_INSTALL_LIBDIR=lib",
                  "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_LUA=ON",
                  "-DMALVADO_BUILD_ASSETS=ON", "-DCMAKE_C_FLAGS_RELEASE=-O3 -UNDEBUG"])
        run(log, ["cmake", "--build", build, "--parallel", "8"])
        catalog = json.loads(run(log, ["ctest", "--test-dir", build, "--show-only=json-v1"]).stdout)
        tests = native_tests(catalog, build)
        remote = "/data/local/tmp/malvado-alpha8-" + uuid.uuid4().hex
        shell(prefix, log, ["mkdir", "-p", remote + "/bin"])
        try:
            with tempfile.TemporaryDirectory(prefix="source payload ", dir=work) as temporary:
                stage = stage_sources(Path(temporary))
                run(log, prefix + ["push", stage, remote + "/source"], timeout=120)
            executables = sorted({test["command"][0] for test in tests})
            run(log, prefix + ["push", *executables, remote + "/bin/"], timeout=120)
            # CMake generates this CRLF fixture for atlas parity verification.
            run(log, prefix + ["push", build / "atlas-pivot-crlf.json", remote + "/bin/"])
            passed = []
            failures = []
            for test in tests:
                settings = {item["name"]: item["value"] for item in test.get("properties", [])}
                command = [remap(value, build, remote) for value in test["command"]]
                cwd = remap(settings.get("WORKING_DIRECTORY", str(build)), build, remote)
                timeout = min(90, max(15, settings.get("TIMEOUT", 30)))
                try:
                    result = shell(prefix, log, command, cwd=cwd,
                                   environment=settings.get("ENVIRONMENT", []), timeout=timeout, check=False)
                    check_result(test, result)
                    passed.append(test["name"])
                except (RuntimeError, subprocess.TimeoutExpired) as error:
                    failures.append(str(error))
                print(f"{config}: {test['name']}: {'PASS' if test['name'] in passed else 'FAIL'}", flush=True)
            verify_installed_sdk(log, work, config, build, ndk, abi, prefix, remote)
            contract_count = verify_lua_contracts(log, work, config, build, prefix, remote)
            summary = {"lua_contracts": contract_count, "installed_sdk": "passed", "configuration": config, "abi": abi, "passed": passed, "failures": failures,
                       "source_revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT).decode().strip(),
                       "dirty": bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT))}
            (work / f"{config}-results.json").write_text(json.dumps(summary, indent=2) + "\n")
            if failures:
                raise RuntimeError("\n".join(failures))
        finally:
            # This exact random path was created by this invocation; never clean another test/device directory.
            shell(prefix, log, ["rm", "-rf", remote], timeout=30)
    print(f"PASS: Android {abi} {config}: {len(passed)} native C/Lua checks", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, default=os.environ.get("ANDROID_SDK_ROOT") or os.environ.get("ANDROID_HOME"))
    parser.add_argument("--serial", required=True, help="Explicit adb serial of a booted API 34 emulator")
    parser.add_argument("--work", type=Path, default=ROOT / "build-android")
    parser.add_argument("--preflight", action="store_true", help="Validate tools/emulator and record their versions only")
    parser.add_argument("--config", choices=("Debug", "Release"), action="append")
    args = parser.parse_args()
    if not args.sdk:
        parser.error("Set ANDROID_SDK_ROOT or pass --sdk")
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    try:
        with (work / "toolchain.log").open("w", encoding="utf-8") as log:
            ndk, prefix, abi, facts = preflight(args.sdk.resolve(), args.serial, log)
            (work / "toolchain.json").write_text(json.dumps(facts, indent=2) + "\n")
        if not args.preflight:
            for config in args.config or ("Debug", "Release"):
                verify_profile(work, config, ndk, prefix, abi)
    except (RuntimeError, OSError, subprocess.TimeoutExpired) as error:
        print(f"FAIL: {error}\nEvidence: {work}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
