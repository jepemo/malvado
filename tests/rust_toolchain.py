#!/usr/bin/env python3
"""Reject false success in Cargo native-probe refresh verification."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_rust_toolchain as verifier


def result(code=0, message=''):
    return subprocess.CompletedProcess([], code, '', message)


class RefreshTests(unittest.TestCase):
    def test_checks_and_restores_each_selection_without_changing_parent(self):
        environment = {'CMAKE_GENERATOR_PLATFORM': 'pinned', 'KEEP': 'value'}
        responses = [result()] + [item for _ in range(3) for item in (result(1, 'malvado-invalid-selection'), result())]
        with patch.object(verifier.subprocess, 'run', side_effect=responses) as run:
            verifier.verify(io.StringIO(), Path('Cargo.toml'), environment, release=True)
        self.assertEqual(run.call_count, 7)
        expected = dict(environment, CMAKE_GENERATOR='Visual Studio 17 2022' if verifier.os.name == 'nt' else 'Unix Makefiles')
        for index, name in enumerate(('CMAKE_GENERATOR_PLATFORM', 'CMAKE_GENERATOR_TOOLSET', 'CMAKE_GENERATOR_INSTANCE')):
            changed = run.call_args_list[1 + 2 * index].kwargs['env']
            self.assertEqual(changed[name], 'malvado-invalid-selection')
            self.assertEqual(run.call_args_list[2 + 2 * index].kwargs['env'], expected)
        self.assertEqual(environment, {'CMAKE_GENERATOR_PLATFORM': 'pinned', 'KEEP': 'value'})
        self.assertIn('--release', run.call_args.args[0])
        self.assertEqual(run.call_args.kwargs['timeout'], 180)

    def test_cached_success_and_unrelated_errors_cannot_satisfy_rejection(self):
        for unexpected in (result(), result(1, 'device offline')):
            with self.subTest(code=unexpected.returncode), patch.object(verifier.subprocess, 'run', side_effect=[result(), unexpected]):
                with self.assertRaisesRegex(RuntimeError, 'did not revalidate'):
                    verifier.verify(io.StringIO(), Path('Cargo.toml'), {})

    def test_baseline_failure_stops_before_negative_cases(self):
        with patch.object(verifier.subprocess, 'run', return_value=result(1, 'SDK unavailable')) as run:
            with self.assertRaisesRegex(RuntimeError, 'baseline failed'):
                verifier.verify(io.StringIO(), Path('Cargo.toml'), {})
            run.assert_called_once()

    def test_failed_restore_is_not_reported_as_recovery(self):
        with patch.object(verifier.subprocess, 'run', side_effect=[result(), result(1, 'malvado-invalid-selection'), result(1, 'bad cache')]) as run:
            with self.assertRaisesRegex(RuntimeError, 'did not recover'):
                verifier.verify(io.StringIO(), Path('Cargo.toml'), {})
            self.assertEqual(run.call_count, 3)


if __name__ == '__main__':
    unittest.main()
