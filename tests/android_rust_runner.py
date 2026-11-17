#!/usr/bin/env python3
"""Verify Rust runner status, output and cleanup on success and failure."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_rust_runner as runner


class RunnerTests(unittest.TestCase):
    def test_output_exit_and_owned_cleanup(self):
        with tempfile.NamedTemporaryFile() as program:
            for code in (0, 7):
                result = subprocess.CompletedProcess([], code, '1 2 3\n')
                with patch.object(runner, 'shell', return_value=result) as shell, patch.object(runner, 'run') as run:
                    self.assertIs(runner.execute('adb with spaces', 'emulator-5580', program.name, ['argument with spaces'], io.StringIO()), result)
                calls = shell.call_args_list
                remote = calls[0].args[2][-1]
                self.assertTrue(remote.startswith('/data/local/tmp/malvado-rust-'))
                self.assertEqual(calls[-1].args[2], ['rm', '-rf', remote])
                self.assertEqual(calls[1].kwargs['environment'], ['TMPDIR=' + remote])
                self.assertEqual(calls[1].args[2], [remote + '/program', 'argument with spaces'])
                self.assertEqual(run.call_args.args[1][-1], remote + '/program')

    def test_push_failure_still_removes_only_owned_directory(self):
        with tempfile.NamedTemporaryFile() as program, patch.object(runner, 'shell') as shell, patch.object(runner, 'run', side_effect=RuntimeError('offline')):
            with self.assertRaisesRegex(RuntimeError, 'offline'):
                runner.execute('adb', 'emulator-1', program.name, [], io.StringIO())
            self.assertEqual(shell.call_args_list[-1].args[2], ['rm', '-rf', shell.call_args_list[0].args[2][-1]])

    def test_inputs_use_owned_root_and_are_removed_after_test_failure(self):
        with tempfile.NamedTemporaryFile() as program, tempfile.TemporaryDirectory(prefix='inputs with spaces') as inputs:
            with patch.object(runner, 'shell', return_value=subprocess.CompletedProcess([], 9, 'failed')) as shell, patch.object(runner, 'run') as run:
                result = runner.execute('adb', 'emulator-1', program.name, [], io.StringIO(), inputs=inputs)
            remote = shell.call_args_list[0].args[2][-1]
            self.assertEqual(result.returncode, 9)
            self.assertEqual(run.call_args.args[1][-2:], [Path(inputs).resolve(), remote + '/inputs'])
            self.assertIn('MALVADO_TEST_ROOT=' + remote + '/inputs', shell.call_args_list[1].kwargs['environment'])
            self.assertEqual(shell.call_args.args[2], ['rm', '-rf', remote])

    def test_symlink_inputs_are_rejected_and_owned_staging_is_removed(self):
        with tempfile.NamedTemporaryFile() as program, tempfile.TemporaryDirectory() as inputs:
            (Path(inputs) / 'link').symlink_to(program.name)
            with patch.object(runner, 'shell') as shell, patch.object(runner, 'run') as run:
                with self.assertRaisesRegex(ValueError, 'without symlinks'):
                    runner.execute('adb', 'emulator-1', program.name, [], io.StringIO(), inputs=inputs)
            self.assertEqual(run.call_count, 1)
            self.assertEqual(shell.call_args.args[2][:2], ['rm', '-rf'])

    def test_transient_fork_memory_pressure_retries_only_directory_preparation(self):
        failure = RuntimeError('Android command transport failed: error: fork failed: Out of memory')
        log = io.StringIO()
        with patch.object(runner, 'shell', side_effect=[failure, None]) as shell, patch.object(runner.time, 'monotonic', side_effect=[0, 0, 0, 1]), patch.object(runner.time, 'sleep') as sleep:
            runner.create_remote(['adb'], log, '/owned')
        self.assertEqual([call.args[2] for call in shell.call_args_list], [['mkdir', '-p', '/owned']] * 2)
        self.assertEqual([call.kwargs['timeout'] for call in shell.call_args_list], [10, 10])
        sleep.assert_called_once_with(1)
        self.assertIn('fork memory pressure', log.getvalue())

    def test_fork_memory_deadline_is_bounded(self):
        failure = RuntimeError('Android command transport failed: error: fork failed: Out of memory')
        with patch.object(runner, 'shell', side_effect=failure) as shell, patch.object(runner.time, 'monotonic', side_effect=[0, 0, 30]), patch.object(runner.time, 'sleep') as sleep:
            with self.assertRaisesRegex(RuntimeError, '30-second'):
                runner.create_remote(['adb'], io.StringIO(), '/owned')
        shell.assert_called_once()
        sleep.assert_not_called()

    def test_other_preparation_errors_are_not_retried(self):
        for message in ('permission denied', 'device offline', 'Android command exited 1'):
            with self.subTest(message=message), patch.object(runner, 'shell', side_effect=RuntimeError(message)) as shell, patch.object(runner.time, 'sleep') as sleep:
                with self.assertRaisesRegex(RuntimeError, message):
                    runner.create_remote(['adb'], io.StringIO(), '/owned')
                shell.assert_called_once()
                sleep.assert_not_called()

    def test_program_memory_failure_is_returned_without_replay(self):
        failure = subprocess.CompletedProcess([], 137, 'error: fork failed: Out of memory')
        with tempfile.NamedTemporaryFile() as program, patch.object(runner, 'shell', side_effect=[None, failure, None]) as shell, patch.object(runner, 'run'), patch.object(runner.time, 'sleep') as sleep:
            self.assertIs(runner.execute('adb', 'emulator-1', program.name, [], io.StringIO()), failure)
            self.assertEqual(shell.call_count, 3)
            sleep.assert_not_called()

    def test_invalid_selection_never_invokes_transport(self):
        with patch.object(runner, 'shell') as shell:
            with self.assertRaises(ValueError):
                runner.execute('adb', 'device', 'missing', [], io.StringIO())
            shell.assert_not_called()


if __name__ == '__main__':
    unittest.main()
