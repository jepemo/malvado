#!/usr/bin/env python3
"""Guard lifecycle evidence against unknown Activity state and process recreation."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch, MagicMock
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class LifecycleTests(unittest.TestCase):
    def test_background_requires_known_resumed_activity_fields(self):
        package = 'dev.malvado.test'
        self.assertFalse(verifier.activity_is_background('', package))
        self.assertFalse(verifier.activity_is_background('unknown format', package))
        self.assertFalse(verifier.activity_is_background('mResumedActivity: ' + package, package))
        self.assertFalse(verifier.activity_is_background('mResumedActivity: launcher\ntopResumedActivity=' + package, package))
        self.assertTrue(verifier.activity_is_background('mResumedActivity: launcher\nold activity: ' + package, package))

    def test_recreated_process_cannot_satisfy_resume_or_trigger_back(self):
        commands = []
        pids = iter(('123\n', '456\n'))
        def run(log, arguments, **kwargs):
            commands.append(arguments)
            output = next(pids) if 'pidof' in arguments else 'mResumedActivity: launcher\n'
            return subprocess.CompletedProcess(arguments, 0, output)
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker, patch.object(verifier.time, 'sleep'):
            with self.assertRaisesRegex(RuntimeError, 'recreated'):
                verifier.lifecycle_actions(io.StringIO(), ['adb'], 'dev.malvado.test', 'token', 'resume')
        marker.assert_called_once()
        self.assertFalse(any(command[-2:] == ['keyevent', '4'] for command in commands))

    def test_lua_keyboard_needs_its_own_confirmation_before_next_key_or_back(self):
        commands = []
        def run(log, arguments, **kwargs):
            commands.append(arguments)
            return subprocess.CompletedProcess(arguments, 0, '123\n')
        def marker(log, adb, message):
            if message.endswith('lua keyboard a released'):
                raise RuntimeError('Lua input not observed')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker', side_effect=marker), patch.object(verifier, 'capture_screen', return_value=(1920,1080)):
            with self.assertRaisesRegex(RuntimeError, 'Lua input not observed'):
                verifier.lifecycle_actions(io.StringIO(), ['adb'], 'package', 'token', 'keyboard', Path('owned'), lua=True)
        self.assertTrue(any(command[-2:] == ['--longpress', '29'] for command in commands))
        self.assertFalse(any(command[-1] in ('21', '4') for command in commands))

    def test_rust_keyboard_needs_its_own_confirmation_before_next_key_or_back(self):
        commands = []
        def run(log, arguments, **kwargs):
            commands.append(arguments)
            return subprocess.CompletedProcess(arguments, 0, '123\n')
        def marker(log, adb, message):
            if message.endswith('rust keyboard a released'):
                raise RuntimeError('Rust input not observed')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker', side_effect=marker), patch.object(verifier, 'capture_screen', return_value=(1920,1080)):
            with self.assertRaisesRegex(RuntimeError, 'Rust input not observed'):
                verifier.lifecycle_actions(io.StringIO(), ['adb'], 'package', 'token', 'keyboard', Path('owned'), rust=True)
        self.assertFalse(any(command[-1] in ('21', '4') for command in commands))

    def test_rust_touch_needs_its_own_release_before_back(self):
        commands = []
        def run(log, arguments, **kwargs):
            commands.append(arguments)
            return subprocess.CompletedProcess(arguments, 0, '123\n')
        def marker(log, adb, message):
            if message.endswith('rust touch released'):
                raise RuntimeError('Rust release not observed')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker', side_effect=marker), patch.object(verifier, 'capture_screen', return_value=(1920,1080)):
            with self.assertRaisesRegex(RuntimeError, 'Rust release not observed'):
                verifier.lifecycle_actions(io.StringIO(), ['adb'], 'package', 'token', 'touch', Path('owned'), rust=True)
        self.assertFalse(any(command[-2:] == ['keyevent', '4'] for command in commands))

    def injection(self, polls):
        process = MagicMock()
        process.poll.side_effect = polls
        process.returncode = 0
        process.communicate.return_value = (b'', None)
        process.__enter__.return_value = process
        return process

    def test_background_key_requires_a_new_numbered_resume_and_same_pid(self):
        process = self.injection([None, None, 0])
        def run(log, args, **kwargs):
            return subprocess.CompletedProcess(args, 0, '123' if 'pidof' in args else 'mResumedActivity: launcher')
        with patch.object(verifier.subprocess, 'Popen', return_value=process), patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker, patch.object(verifier, 'capture_screen'):
            self.assertEqual(verifier.background_key(io.StringIO(), ['adb'], 'package', 'token', '21', 'left', '123', Path('owned'), cycle=2, rust=True), '123')
        self.assertIn('token lifecycle resumed 2', [call.args[2] for call in marker.call_args_list])
        self.assertIn('token rust keyboard left held', [call.args[2] for call in marker.call_args_list])
        process.kill.assert_not_called()

    def test_background_key_does_not_accept_injection_that_already_finished(self):
        process = self.injection([0, 0])
        with patch.object(verifier.subprocess, 'Popen', return_value=process), patch.object(verifier, 'run') as run, patch.object(verifier, 'wait_marker'):
            with self.assertRaisesRegex(RuntimeError, 'ended before background'):
                verifier.background_key(io.StringIO(), ['adb'], 'package', 'token', '29', 'a', '123', None)
        run.assert_not_called()

    def test_missing_held_marker_stops_the_owned_injection(self):
        process = self.injection([None])
        with patch.object(verifier.subprocess, 'Popen', return_value=process), patch.object(verifier, 'wait_marker', side_effect=RuntimeError('missing held')):
            with self.assertRaisesRegex(RuntimeError, 'missing held'):
                verifier.background_key(io.StringIO(), ['adb'], 'package', 'token', '29', 'a', '123', None)
        process.kill.assert_called_once()
        process.communicate.assert_called_once_with(timeout=5)

    def test_background_touch_does_not_run_keyboard_injection(self):
        commands = []
        def run(log, args, **kwargs):
            commands.append(args)
            return subprocess.CompletedProcess(args, 0, '123')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker, patch.object(verifier, 'capture_screen', return_value=(1920,1080)), patch.object(verifier, 'background_touch', return_value='123') as touch:
            facts = verifier.lifecycle_actions(io.StringIO(), ['adb'], 'package', 'token', 'touch-background', Path('owned'), rust=True)
        touch.assert_called_once()
        self.assertTrue(facts['background_foreground'] and facts['release_while_backgrounded'])
        self.assertIsNone(facts['touch_gesture']['duration_ms'])
        self.assertFalse(any('keyboard' in value.args[2] for value in marker.call_args_list))
        self.assertEqual(commands[-1][-2:], ['keyevent', '4'])

    def test_background_touch_failure_releases_the_owned_contact(self):
        commands = []
        def run(log, args, **kwargs):
            commands.append(args)
            return subprocess.CompletedProcess(args, 0, '')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker', side_effect=RuntimeError('missing held')):
            with self.assertRaisesRegex(RuntimeError, 'missing held'):
                verifier.background_touch(io.StringIO(), ['adb'], 'package', 'token', '123', (1920,1080))
        self.assertEqual(commands[0][-3:], ['DOWN', '960', '540'])
        self.assertEqual(commands[-1][-3:], ['UP', '1152', '540'])
        self.assertFalse(any('start' in args for args in commands))

    def test_background_touch_rejects_process_recreation(self):
        def run(log, args, **kwargs):
            return subprocess.CompletedProcess(args, 0, '456' if 'pidof' in args else 'mResumedActivity: launcher')
        with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker:
            with self.assertRaisesRegex(RuntimeError, 'recreated'):
                verifier.background_touch(io.StringIO(), ['adb'], 'package', 'token', '123', (1920,1080))
        self.assertFalse(any('lifecycle resumed' in call.args[2] for call in marker.call_args_list))


if __name__ == '__main__':
    unittest.main()
