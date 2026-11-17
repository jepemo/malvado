#!/usr/bin/env python3
"""Check fixed Windows selection, metadata rejection and preflight cleanup."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import windows_toolchain as toolchain


class ToolchainTests(unittest.TestCase):
    def instance(self, root):
        instance = root / 'VS with spaces'
        for name in (f'VC/Tools/MSVC/{toolchain.MSVC}/bin/Hostx64/x64/cl.exe', 'MSBuild/Current/Bin/MSBuild.exe'):
            path = instance / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.touch()
        return instance

    def facts(self, instance):
        return dict(generator=toolchain.GENERATOR, platform=toolchain.PLATFORM,
                    toolset=toolchain.TOOLSET, instance=str(instance), vs_build=toolchain.VS_BUILD,
                    compiler=str(instance / f'VC/Tools/MSVC/{toolchain.MSVC}/bin/Hostx64/x64/cl.exe'),
                    compiler_id='MSVC', compiler_version=toolchain.COMPILER, sdk=toolchain.SDK)

    def cache(self, build, environment):
        (build / 'CMakeCache.txt').write_text(''.join(f'{name}:INTERNAL={value}\n' for name, value in environment.items() if name.startswith('CMAKE_GENERATOR')))

    def test_selects_pinned_complete_instance_instead_of_latest(self):
        with tempfile.TemporaryDirectory() as temporary:
            instance = self.instance(Path(temporary))
            record = dict(installationVersion=toolchain.VS_BUILD, installationPath=str(instance))
            self.assertEqual(toolchain.select_instance([dict(record, installationVersion='18.0.1'), record]), instance)
            for records in ([], [{}], [dict(record, installationVersion='17.14.0')]):
                with self.assertRaisesRegex(RuntimeError, 'no matching instance'):
                    toolchain.select_instance(records)
            (instance / 'MSBuild/Current/Bin/MSBuild.exe').unlink()
            with self.assertRaises(RuntimeError):
                toolchain.select_instance([record])

    def test_selection_overrides_conflicts_without_mutating_parent(self):
        base = dict(CMAKE_GENERATOR='Ninja', CMAKE_GENERATOR_TOOLSET='other', KEEP='value')
        selected = toolchain.selection_environment(Path('VS'), base)
        self.assertEqual(selected['CMAKE_GENERATOR'], toolchain.GENERATOR)
        self.assertEqual(selected['CMAKE_GENERATOR_PLATFORM'], toolchain.PLATFORM)
        self.assertEqual(selected['KEEP'], 'value')
        self.assertEqual(base['CMAKE_GENERATOR'], 'Ninja')

    def test_detects_compiler_sdk_generator_and_instance_mismatch(self):
        instance = Path('VS')
        facts = self.facts(instance)
        toolchain.validate_selection(facts, instance)
        for name in facts:
            with self.subTest(name=name), self.assertRaisesRegex(RuntimeError, 'mismatch'):
                toolchain.validate_selection(dict(facts, **{name: 'different'}), instance)

    def test_old_cached_default_cannot_bypass_pins(self):
        with tempfile.TemporaryDirectory() as temporary:
            build = Path(temporary)
            env = toolchain.selection_environment(Path('VS'), {})
            self.cache(build, env)
            toolchain.validate_cache(build, env)
            self.cache(build, dict(env, CMAKE_GENERATOR_TOOLSET=''))
            with self.assertRaisesRegex(RuntimeError, 'new --work directory'):
                toolchain.validate_cache(build, env)

    def prepare_fixture(self, root, *, fail=False, missing_sdk=False):
        instance = self.instance(root)
        environment = {'ProgramFiles(x86)': str(root), 'KEEP': 'original'}
        if not missing_sdk:
            for name in (f'Include/{toolchain.SDK}/um/Windows.h', f'Include/{toolchain.SDK}/ucrt/stdlib.h', f'Lib/{toolchain.SDK}/um/x64/kernel32.lib'):
                path = root / 'Windows Kits/10' / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.touch()
        calls, builds = [], []
        def run(command, **kwargs):
            calls.append((command, kwargs))
            if '-format' in command:
                return subprocess.CompletedProcess(command, 0, json.dumps([dict(installationPath=str(instance), installationVersion=toolchain.VS_BUILD)]))
            if '-S' in command:
                build = Path(command[-1])
                builds.append(build)
                if fail:
                    raise subprocess.CalledProcessError(1, command)
                (build / 'selection.txt').write_text(''.join(f'{k}={v}\n' for k, v in self.facts(instance).items()))
                self.cache(build, kwargs['env'])
                for config in ('Debug', 'Release'):
                    (build / f'program-{config}.txt').write_text(str(build / f'probe-{config}.exe'))
            return subprocess.CompletedProcess(command, 0, 'Windows toolchain probe passed\n', '')
        return environment, calls, builds, run

    def test_preflight_runs_both_profiles_records_proof_and_cleans_own_tree(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            environment, calls, builds, run = self.prepare_fixture(root)
            with patch.object(toolchain, 'os', SimpleNamespace(name='nt', environ=environment)), patch.object(toolchain.subprocess, 'run', side_effect=run):
                selected = toolchain.prepare(root)
            facts = json.loads((root / 'windows-toolchain.json').read_text())
            self.assertEqual(facts['compiler_version'], toolchain.COMPILER)
            self.assertEqual(len(calls), 6)  # Discovery, configure, two builds and two executions.
            for command, kwargs in calls[1:]:
                self.assertEqual(kwargs['env'], selected)
                self.assertLessEqual(kwargs['timeout'], 90)
            self.assertFalse(builds[0].exists())
            self.assertEqual(environment, {'ProgramFiles(x86)': str(root), 'KEEP': 'original'})

    def test_preflight_failure_retains_log_without_publishing_success(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            environment, calls, builds, run = self.prepare_fixture(root, fail=True)
            (root / "windows-toolchain.json").write_text("stale success")
            with patch.object(toolchain, 'os', SimpleNamespace(name='nt', environ=environment)), patch.object(toolchain.subprocess, 'run', side_effect=run):
                with self.assertRaises(subprocess.CalledProcessError):
                    toolchain.prepare(root)
            self.assertTrue((root / 'windows-toolchain.log').is_file())
            self.assertFalse((root / 'windows-toolchain.json').exists())
            self.assertFalse(builds[0].exists())
            self.assertEqual(len(calls), 2)

    def test_missing_sdk_stops_before_any_compiler_runs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            environment, calls, builds, run = self.prepare_fixture(root, missing_sdk=True)
            with patch.object(toolchain, 'os', SimpleNamespace(name='nt', environ=environment)), patch.object(toolchain.subprocess, 'run', side_effect=run):
                with self.assertRaisesRegex(RuntimeError, 'Install Windows SDK'):
                    toolchain.prepare(root)
            self.assertEqual(len(calls), 1)
            self.assertEqual(builds, [])


if __name__ == '__main__':
    unittest.main()
