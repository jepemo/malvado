#!/usr/bin/env python3
"""Require real process death and PID-scoped cold-restart evidence."""
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


class RestartTests(unittest.TestCase):
    def actions(self, pids):
        values = iter(pids)
        commands = []
        def run(log, args, **kwargs):
            commands.append(args)
            return subprocess.CompletedProcess(args, 0, next(values) if 'pidof' in args else '')
        return commands, run

    def test_new_process_evidence_and_cleanup(self):
        commands, run = self.actions(('123', '', '456'))
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker, patch.object(verifier, 'capture_screen'):
            result = verifier.restart_actions(io.StringIO(), ['adb'], 'owned', 'token', Path('work'), 'passed')
        self.assertEqual(result['restarted_pid'], '456')
        self.assertEqual(marker.call_args_list[1].kwargs, {'pid': '456'})
        self.assertEqual(marker.call_args_list[2].kwargs, {'pid': '456'})
        self.assertIn(['adb', 'shell', 'am', 'force-stop', 'owned'], commands)

    def test_surviving_missing_or_reused_pid_rejected_before_back(self):
        for pids in (('123', '123'), ('123', '', '123'), ('123', '', ''), ('123 456',)):
            commands, run = self.actions(pids)
            with self.subTest(pids=pids), patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker'), patch.object(verifier, 'capture_screen'):
                with self.assertRaises(RuntimeError):
                    verifier.restart_actions(io.StringIO(), ['adb'], 'owned', 'token', Path('work'), 'passed')
            self.assertFalse(any(args[-2:] == ['keyevent', '4'] for args in commands))

    def test_marker_query_filters_by_new_pid(self):
        with patch.object(verifier, 'run', return_value=subprocess.CompletedProcess([], 0, 'token ready')) as run:
            verifier.wait_marker(io.StringIO(), ['adb'], 'token ready', pid='456')
        self.assertIn('--pid=456', run.call_args.args[1])

    def test_stale_ready_cannot_trigger_back(self):
        commands, run = self.actions(('123', '', '456'))
        def marker(*args, **kwargs):
            if kwargs.get('pid'):
                raise RuntimeError('new process never rendered')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker', side_effect=marker), patch.object(verifier, 'capture_screen'):
            with self.assertRaisesRegex(RuntimeError, 'never rendered'):
                verifier.restart_actions(io.StringIO(), ['adb'], 'owned', 'token', Path('work'), 'passed')
        self.assertFalse(any(args[-2:] == ['keyevent', '4'] for args in commands))


if __name__ == '__main__':
    unittest.main()
