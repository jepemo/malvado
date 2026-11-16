#!/usr/bin/env python3
"""Guard lifecycle evidence against unknown Activity state and process recreation."""
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


if __name__ == '__main__':
    unittest.main()
