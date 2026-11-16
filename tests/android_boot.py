#!/usr/bin/env python3
"""Check bounded Android boot polling, transient failures and selection."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_boot


class BootTests(unittest.TestCase):
    def test_transient_failure_then_positive_confirmation(self):
        outcomes = [subprocess.TimeoutExpired('adb', 10),
                    subprocess.CompletedProcess([], 1, '1', 'offline'),
                    subprocess.CompletedProcess([], 0, '0', ''),
                    subprocess.CompletedProcess([], 0, '1\n', '')]
        with patch.object(android_boot.subprocess, 'run', side_effect=outcomes) as run, patch.object(android_boot.time, 'sleep'):
            android_boot.wait('SDK with spaces/adb', 'emulator-5582')
        self.assertEqual(run.call_count, 4)
        self.assertEqual(run.call_args.args[0], ['SDK with spaces/adb', '-s', 'emulator-5582', 'shell', 'getprop', 'sys.boot_completed'])

    def test_deadline_bounds_command_and_sleep_and_reports_last_error(self):
        with patch.object(android_boot.time, 'monotonic', side_effect=[0, 0, 0.5, 1]), patch.object(android_boot.time, 'sleep') as sleep, patch.object(android_boot.subprocess, 'run', return_value=subprocess.CompletedProcess([], 1, '', 'offline')) as run:
            with self.assertRaisesRegex(RuntimeError, 'emulator-5582.*offline'):
                android_boot.wait('adb', 'emulator-5582', 1)
        self.assertEqual(run.call_args.kwargs['timeout'], 1)
        sleep.assert_called_once_with(0.5)

    def test_invalid_selection_or_timeout_never_runs_adb(self):
        with patch.object(android_boot.subprocess, 'run') as run:
            for serial, timeout in [('device', 1), ('emulator-', 1), ('emulator-1', 0), ('emulator-1', float('inf')), ('emulator-1', float('nan'))]:
                with self.subTest(serial=serial, timeout=timeout), self.assertRaises(ValueError):
                    android_boot.wait('adb', serial, timeout)
        run.assert_not_called()


if __name__ == '__main__':
    unittest.main()
