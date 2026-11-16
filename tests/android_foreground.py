#!/usr/bin/env python3
"""Verify foreground prerequisites and best-effort Android failure evidence."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class ForegroundTests(unittest.TestCase):
    def test_wakeup_is_not_a_power_toggle(self):
        with patch.object(verifier, 'run') as run:
            verifier.prepare_foreground(io.StringIO(), ['adb', '-s', 'emulator-test'])
        self.assertEqual([call.args[1] for call in run.call_args_list], [
            ['adb', '-s', 'emulator-test', 'shell', 'input', 'keyevent', '224'],
            ['adb', '-s', 'emulator-test', 'shell', 'wm', 'dismiss-keyguard'],
        ])

    def test_preparation_failure_is_not_ignored(self):
        with patch.object(verifier, 'run', side_effect=RuntimeError('device offline')):
            with self.assertRaisesRegex(RuntimeError, 'offline'):
                verifier.prepare_foreground(io.StringIO(), ['adb'])

    def test_evidence_failure_does_not_hide_acceptance_error(self):
        log = io.StringIO()
        with patch.object(verifier, 'run', side_effect=subprocess.TimeoutExpired('adb', 10)) as run, patch.object(verifier, 'capture_screen', side_effect=ValueError('invalid PNG')) as capture:
            verifier.failure_evidence(log, ['adb'], Path('owned-work'))
        self.assertEqual(run.call_count, 4)
        capture.assert_called_once_with(log, ['adb'], Path('owned-work/failure.png'))
        self.assertIn('Cannot collect Android state', log.getvalue())
        self.assertIn('Cannot capture Android failure screen', log.getvalue())


if __name__ == '__main__':
    unittest.main()
