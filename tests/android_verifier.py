#!/usr/bin/env python3
"""Host-independent regressions for Android verification boundaries."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import test_android as verifier
import test_android_ime as ime


class AndroidVerifierTests(unittest.TestCase):
    def test_ime_waits_for_exact_component(self):
        component = 'test.package/test.InputMethod'
        command = Mock(side_effect=[component + 'Other', component])
        with patch.object(ime.time, 'sleep') as sleep:
            ime.wait_for_ime(command, component)
        self.assertEqual(command.call_count, 2)
        command.assert_called_with(['ime', 'list', '-a', '-s'])
        sleep.assert_called_once()

    def test_missing_ime_has_bounded_failure(self):
        component = 'test.package/test.InputMethod'
        with patch.object(ime.time, 'monotonic', side_effect=[0, 1, 16]), \
                patch.object(ime.time, 'sleep'), self.assertRaisesRegex(RuntimeError, component):
            ime.wait_for_ime(Mock(return_value=''), component)

    def test_ndk_properties_allow_official_spacing(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "source.properties"
            path.write_text("# Package\nPkg.Revision = 27.2.12479018\nPkg.Desc=Android NDK\n")
            self.assertEqual(verifier.properties(path)["Pkg.Revision"], verifier.NDK_VERSION)

    def test_preflight_records_runtime_archive_provenance(self):
        with tempfile.TemporaryDirectory() as temporary:
            sdk = Path(temporary)
            for name, text in ((f'ndk/{verifier.NDK_VERSION}/source.properties', f'Pkg.Revision={verifier.NDK_VERSION}'),
                               ('platform-tools/source.properties', 'Pkg.Revision=37.0.1'),
                               ('emulator/source.properties', 'Pkg.Revision=37.2.13')):
                path = sdk / name
                path.parent.mkdir(parents=True)
                path.write_text(text)
            provenance = dict(host='macos', abi='arm64-v8a', archives=[dict(archive='pinned.zip', sha256='1' * 64)])
            (sdk / 'malvado-emulator-archives.json').write_text(json.dumps(provenance))
            properties = {'ro.kernel.qemu': '1', 'ro.product.cpu.abi': 'arm64-v8a',
                          'ro.build.version.sdk': '34', 'sys.boot_completed': '1'}
            def run(log, arguments, **kwargs):
                value = properties.get(arguments[-1], 'fixture')
                return subprocess.CompletedProcess(arguments, 0, value)
            with patch.object(verifier.platform, 'system', return_value='Darwin'), patch.object(verifier, 'run', side_effect=run):
                _, _, abi, facts = verifier.preflight(sdk, 'emulator-5584', io.StringIO())
            self.assertEqual(abi, 'arm64-v8a')
            self.assertEqual(facts['emulator_archive_inputs'], provenance)

    def test_remote_shell_quotes_paths_and_arguments(self):
        arguments = ["/game with spaces/run", "", "a'b", "$(touch forbidden)", "semi;colon"]
        with patch.object(verifier.uuid, "uuid4") as identifier, patch.object(verifier, "run") as run:
            identifier.return_value.hex = "fixture"
            run.return_value = subprocess.CompletedProcess([], 0, "\nMALVADO_ANDROID_EXIT_fixture:0\n")
            verifier.shell(["adb", "-s", "emulator-5580"], io.StringIO(), arguments,
                           environment=["VALUE=space and $dollar"])
        command = run.call_args.args[1]
        self.assertEqual(command[:4], ["adb", "-s", "emulator-5580", "shell"])
        self.assertEqual(shlex.split(command[4].split("; result=$?;", 1)[0]),
                         ["env", "VALUE=space and $dollar", *arguments, "2>&1"])

    @unittest.skipUnless(shutil.which("sh"), "requires a POSIX shell")
    def test_remote_diagnostics_arrive_before_exit_marker(self):
        def drain_stdout_first(log, arguments, **kwargs):
            result = subprocess.run(["sh", "-c", arguments[-1]], text=True, capture_output=True)
            return subprocess.CompletedProcess(arguments, result.returncode, result.stdout + result.stderr)

        with patch.object(verifier, "run", side_effect=drain_stdout_first):
            result = verifier.shell(["adb"], io.StringIO(),
                                    ["sh", "-c", "echo runner; echo nested runner rejected >&2; exit 2"],
                                    check=False)
        self.assertEqual(result.returncode, 2)
        self.assertEqual(result.stdout, "runner\nnested runner rejected\n")

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
