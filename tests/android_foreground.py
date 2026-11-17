#!/usr/bin/env python3
"""Verify foreground prerequisites and best-effort Android failure evidence."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import unittest
import xml.etree.ElementTree as ET
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class ForegroundTests(unittest.TestCase):
    def test_native_activity_handles_all_size_configuration_changes(self):
        root = ET.fromstring(verifier.android_manifest('test.game'))
        activity = root.find('application/activity')
        flags = activity.get('{http://schemas.android.com/apk/res/android}configChanges').split('|')
        self.assertTrue({'orientation', 'screenSize', 'screenLayout', 'smallestScreenSize'}.issubset(flags))
        self.assertIn('keyboardHidden', flags)

    def test_startup_cancel_activity_has_java_code_and_native_entry(self):
        root = ET.fromstring(verifier.android_manifest('test.game', True))
        ns = '{http://schemas.android.com/apk/res/android}'
        application = root.find('application')
        activity = application.find('activity')
        self.assertEqual(application.get(ns + 'hasCode'), 'true')
        self.assertEqual(activity.get(ns + 'name'), 'dev.malvado.test.CancelActivity')
        self.assertEqual(activity.find('meta-data').get(ns + 'value'), 'game')
        self.assertEqual(ET.fromstring(verifier.android_manifest('test.game')).find('application').get(ns + 'hasCode'), 'false')

    def test_startup_cancel_dex_uses_selected_sdk_and_owns_output(self):
        import tempfile
        with tempfile.TemporaryDirectory() as directory, patch.object(verifier, 'run') as run:
            work = Path(directory)
            class_file = work / 'classes/dev/malvado/test/CancelActivity.class'
            def compile_class(log, command, **kwargs):
                if command[0] == 'javac':
                    class_file.parent.mkdir(parents=True)
                    class_file.touch()
            run.side_effect = compile_class
            result = verifier.cancellation_dex(io.StringIO(), Path('sdk'), Path('tools'), work)
            commands = [call.args[1] for call in run.call_args_list]
            self.assertEqual(result, work / 'dex/classes.dex')
            self.assertIn(Path('sdk/platforms/android-34/android.jar'), commands[0])
            self.assertEqual(commands[1][0], Path('tools/d8'))
            self.assertEqual(commands[1][-1], work / 'classes/dev/malvado/test/CancelActivity.class')

    def test_recreation_manifest_and_dex_include_receiver_class(self):
        import tempfile
        ns = '{http://schemas.android.com/apk/res/android}'
        root = ET.fromstring(verifier.android_manifest('test.game', recreate=True))
        self.assertEqual(root.find('application').get(ns + 'hasCode'), 'true')
        self.assertEqual(root.find('application/activity').get(ns + 'name'), 'dev.malvado.test.RecreateActivity')
        with tempfile.TemporaryDirectory() as directory, patch.object(verifier, 'run') as run:
            work = Path(directory)
            classes = work / 'classes/dev/malvado/test'
            classes.mkdir(parents=True)
            (classes / 'RecreateActivity$1.class').touch()
            def compile_classes(log, command, **kwargs):
                if command[0] == 'javac':
                    classes.mkdir(parents=True)
                    for name in ('RecreateActivity.class', 'RecreateActivity$Receiver.class'):
                        (classes / name).touch()
            run.side_effect = compile_classes
            verifier.cancellation_dex(io.StringIO(), Path('sdk'), Path('tools'), work, 'RecreateActivity')
            command = run.call_args_list[-1].args[1]
            self.assertIn(classes / 'RecreateActivity.class', command)
            self.assertIn(classes / 'RecreateActivity$Receiver.class', command)
            self.assertNotIn(classes / 'RecreateActivity$1.class', command)
            self.assertNotIn(classes / 'CancelActivity.class', command)

    def test_wakeup_is_not_a_power_toggle(self):
        with patch.object(verifier, 'run') as run:
            verifier.prepare_foreground(io.StringIO(), ['adb', '-s', 'emulator-test'])
        self.assertEqual([call.args[1] for call in run.call_args_list], [
            ['adb', '-s', 'emulator-test', 'shell', 'input', 'keyevent', '224'],
            ['adb', '-s', 'emulator-test', 'shell', 'wm', 'dismiss-keyguard'],
        ])

    def test_installation_requires_a_positive_idle_confirmation(self):
        for output in ('All broadcast queues are idle!', 'Unknown command', ''):
            with self.subTest(output=output), patch.object(verifier, 'run', return_value=subprocess.CompletedProcess([], 0, output)):
                if output.startswith('All broadcast'):
                    verifier.settle_installation(io.StringIO(), ['adb'])
                else:
                    with self.assertRaisesRegex(RuntimeError, 'broadcasts did not reach idle'):
                        verifier.settle_installation(io.StringIO(), ['adb'])

    def test_preparation_failure_is_not_ignored(self):
        with patch.object(verifier, 'run', side_effect=RuntimeError('device offline')):
            with self.assertRaisesRegex(RuntimeError, 'offline'):
                verifier.prepare_foreground(io.StringIO(), ['adb'])

    def test_evidence_failure_does_not_hide_acceptance_error(self):
        log = io.StringIO()
        with patch.object(verifier, 'run', side_effect=subprocess.TimeoutExpired('adb', 10)) as run, patch.object(verifier, 'capture_screen', side_effect=ValueError('invalid PNG')) as capture:
            verifier.failure_evidence(log, ['adb'], Path('owned-work'))
        self.assertEqual(run.call_count, 5)
        capture.assert_called_once_with(log, ['adb'], Path('owned-work/failure.png'))
        self.assertIn('Cannot collect Android state', log.getvalue())
        self.assertIn('Cannot capture Android failure screen', log.getvalue())

    def test_native_stack_is_limited_to_the_selected_process(self):
        for pid in ('123', '', '123 456', '123;other'):
            with self.subTest(pid=pid):
                def run(log, arguments, **kwargs):
                    return subprocess.CompletedProcess(arguments, 0, pid if 'pidof' in arguments else '')
                with patch.object(verifier, 'run', side_effect=run) as command, patch.object(verifier, 'capture_screen'):
                    verifier.failure_evidence(io.StringIO(), ['adb'], Path('owned'), 'selected.package')
                traces = [call for call in command.call_args_list if 'debuggerd' in call.args[1]]
                self.assertEqual(len(traces), int(pid == '123'))
                if traces:
                    self.assertEqual(traces[0].args[1], ['adb', 'shell', 'debuggerd', '-b', '123'])


if __name__ == '__main__':
    unittest.main()
