#!/usr/bin/env python3
"""Host-independent regressions for Android verification boundaries."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_android as verifier


class AndroidVerifierTests(unittest.TestCase):
    def test_ndk_properties_allow_official_spacing(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "source.properties"
            path.write_text("# Package\nPkg.Revision = 27.2.12479018\nPkg.Desc=Android NDK\n")
            self.assertEqual(verifier.properties(path)["Pkg.Revision"], verifier.NDK_VERSION)

    def test_remote_shell_quotes_paths_and_arguments(self):
        arguments = ["/game with spaces/run", "", "a'b", "$(touch forbidden)", "semi;colon"]
        with patch.object(verifier.uuid, "uuid4") as identifier, patch.object(verifier, "run") as run:
            identifier.return_value.hex = "fixture"
            run.return_value = subprocess.CompletedProcess([], 0, "\nMALVADO_ANDROID_EXIT_fixture:0\n")
            verifier.shell(["adb", "-s", "emulator-5580"], io.StringIO(), arguments,
                           environment=["VALUE=space and $dollar"])
        command = run.call_args.args[1]
        self.assertEqual(command[:4], ["adb", "-s", "emulator-5580", "shell"])
        self.assertEqual(shlex.split(command[4].split("; result=$?;", 1)[0]), ["env", "VALUE=space and $dollar", *arguments])

    def test_transport_failure_cannot_satisfy_expected_program_failure(self):
        with patch.object(verifier, "run") as run:
            run.return_value = subprocess.CompletedProcess([], 1, "adb: device offline\n")
            with self.assertRaisesRegex(RuntimeError, "transport failed"):
                verifier.shell(["adb"], io.StringIO(), ["program"], check=False)

    def test_expected_failure_rejects_success_and_signal_or_shell_failure(self):
        test = {"name": "negative", "properties": [{"name": "WILL_FAIL", "value": True}]}
        verifier.check_result(test, subprocess.CompletedProcess([], 1, "expected error"))
        for status in (0, -11, 126, 127, 134, 139):
            with self.subTest(status=status), self.assertRaises(RuntimeError):
                verifier.check_result(test, subprocess.CompletedProcess([], status, ""))

    def test_success_requires_expected_output(self):
        test = {"name": "trace", "properties": [{"name": "PASS_REGULAR_EXPRESSION", "value": ["tick: 3"]}]}
        verifier.check_result(test, subprocess.CompletedProcess([], 0, "tick: 3\n"))
        with self.assertRaises(RuntimeError):
            verifier.check_result(test, subprocess.CompletedProcess([], 0, ""))

    def test_discovery_excludes_host_commands_and_desktop_cli(self):
        with tempfile.TemporaryDirectory() as temporary:
            build = Path(temporary)
            native = build / "process-test"
            native.write_bytes(b"\x7fELFfixture")
            catalog = {"tests": [{"name": "process", "command": [str(native)]},
                                  {"name": "host", "command": ["cmake", "-P", "test.cmake"]},
                                  {"name": "cli", "command": [str(build / "malvado")]}]}
            self.assertEqual([test["name"] for test in verifier.native_tests(catalog, build)], ["process"])
            native.write_bytes(b"host executable")
            with self.assertRaises(RuntimeError):
                verifier.native_tests(catalog, build)

    def test_remapping_respects_path_boundaries(self):
        build = verifier.ROOT / "build-android" / "Debug"
        self.assertEqual(verifier.remap(str(build / "space name.json"), build, "/owned"),
                         "/owned/bin/space name.json")
        unrelated = str(verifier.ROOT) + "-other/file"
        self.assertEqual(verifier.remap(unrelated, build, "/owned"), unrelated)


if __name__ == "__main__":
    unittest.main()
