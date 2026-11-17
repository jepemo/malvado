#!/usr/bin/env python3
"""Verify the resource-overlay fixture and its activation evidence."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_overlay as overlay
import test_android_window as verifier


class OverlayTests(unittest.TestCase):
    def test_exact_overlay_state(self):
        self.assertTrue(overlay.is_enabled('target\n[x] own.overlay\n', 'own.overlay', True))
        self.assertTrue(overlay.is_enabled('[ ] own.overlay', 'own.overlay', False))
        for output in ('', '[x] own.overlay.other', '--- own.overlay', '[ ] own.overlay'):
            self.assertFalse(overlay.is_enabled(output, 'own.overlay', True))

    def test_target_resources_are_explicitly_overlayable(self):
        with tempfile.TemporaryDirectory() as folder:
            resource = overlay.resources(Path(folder))
            root = ET.parse(resource / 'values/overlayable.xml').getroot()
            self.assertEqual(root.find('overlayable').get('name'), 'Acceptance')
            self.assertEqual(root.find('.//item').attrib, {'type': 'string', 'name': 'app_name'})
            manifest = ET.fromstring(verifier.android_manifest('owned.package', recreate=True, overlay=True))
            self.assertEqual(manifest.find('application').get('{http://schemas.android.com/apk/res/android}label'), '@string/app_name')

    def test_overlay_manifest_targets_only_owned_app(self):
        with tempfile.TemporaryDirectory() as folder:
            calls = []
            name, apk = overlay.build(io.StringIO(), lambda log, args: calls.append(args), Path('tools'), Path('sdk'), Path(folder), 'owned.package', Path('key'))
            root = ET.parse(Path(folder) / 'OverlayManifest.xml').getroot()
            attributes = root.find('overlay').attrib
            namespace = '{http://schemas.android.com/apk/res/android}'
            self.assertEqual(attributes[namespace + 'targetPackage'], 'owned.package')
            self.assertEqual(attributes[namespace + 'targetName'], 'Acceptance')
            self.assertEqual(name, 'owned.package.overlay')
            self.assertEqual(calls[-1][1], 'verify')

    def test_activation_requires_disabled_then_enabled_and_never_broadcasts(self):
        for states in (('[ ] owned.overlay', '[x] owned.overlay'),
                       ('[x] owned.overlay', ''), ('[ ] owned.overlay', '[ ] owned.overlay')):
            outputs = iter(states)
            commands = []
            def run(log, args, **kwargs):
                commands.append(args)
                value = next(outputs) if 'list' in args else ('123' if 'pidof' in args else '')
                return subprocess.CompletedProcess(args, 0, value)
            with patch.object(verifier, 'run', side_effect=run), patch.object(verifier, 'wait_marker') as marker, patch.object(verifier, 'capture_screen'):
                if states[1] == '[x] owned.overlay':
                    result = verifier.recreate_actions(io.StringIO(), ['adb'], 'owned', 'token', Path('work'), overlay='owned.overlay')
                    self.assertEqual(result['resource_overlay'], 'owned.overlay')
                    self.assertIn('token lifecycle cleaned 2', [call.args[2] for call in marker.call_args_list])
                else:
                    with self.assertRaises(RuntimeError):
                        verifier.recreate_actions(io.StringIO(), ['adb'], 'owned', 'token', Path('work'), overlay='owned.overlay')
                    self.assertFalse(any(args[-2:] == ['keyevent', '4'] for args in commands))
            self.assertFalse(any('broadcast' in args for args in commands))


if __name__ == '__main__':
    unittest.main()
