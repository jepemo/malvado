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

    def test_invalid_selection_never_invokes_transport(self):
        with patch.object(runner, 'shell') as shell:
            with self.assertRaises(ValueError):
                runner.execute('adb', 'device', 'missing', [], io.StringIO())
            shell.assert_not_called()


if __name__ == '__main__':
    unittest.main()
