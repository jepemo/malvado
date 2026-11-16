#!/usr/bin/env python3
"""Check binary screenshot capture, input geometry and partial-file cleanup."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import io
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import test_android_window as verifier


class ScreenshotTests(unittest.TestCase):
    def test_dimensions_reject_malformed_or_unsafe_input(self):
        prefix = b'\x89PNG\r\n\x1a\n' + struct.pack('>I', 13) + b'IHDR'
        self.assertEqual(verifier.png_dimensions(prefix + struct.pack('>II', 1920, 1080)), (1920, 1080))
        for header in (b'', b'not a PNG', prefix + struct.pack('>II', 0, 1080),
                       prefix + struct.pack('>II', 1920, 20000),
                       prefix[:8] + b'wronglen' + struct.pack('>II', 1920, 1080)):
            with self.subTest(header=header), self.assertRaises(ValueError):
                verifier.png_dimensions(header)

    def test_capture_preserves_binary_bytes(self):
        payload = (verifier.ROOT / 'tests/fixtures/red.png').read_bytes()
        def run(arguments, **kwargs):
            self.assertEqual(arguments, ['adb', 'exec-out', 'screencap', '-p'])
            self.assertEqual(kwargs['timeout'], 15)
            kwargs['stdout'].write(payload)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'frame.png'
            with patch.object(verifier.subprocess, 'run', side_effect=run):
                self.assertEqual(verifier.capture_screen(io.StringIO(), ['adb'], output), (8, 8))
            self.assertEqual(output.read_bytes(), payload)
            self.assertFalse(output.with_suffix('.tmp').exists())

    def test_failure_removes_partial_capture_and_preserves_previous_file(self):
        for failure in ('invalid', 'timeout'):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as temporary:
                output = Path(temporary) / 'frame.png'
                output.write_bytes(b'previous')
                def run(arguments, **kwargs):
                    kwargs['stdout'].write(b'partial')
                    if failure == 'timeout':
                        raise subprocess.TimeoutExpired(arguments, 15)
                with patch.object(verifier.subprocess, 'run', side_effect=run):
                    with self.assertRaises((ValueError, subprocess.TimeoutExpired)):
                        verifier.capture_screen(io.StringIO(), ['adb'], output)
                self.assertEqual(output.read_bytes(), b'previous')
                self.assertFalse(output.with_suffix('.tmp').exists())


if __name__ == '__main__':
    unittest.main()
