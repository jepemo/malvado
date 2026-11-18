#!/usr/bin/env python3
"""Check the candidate site built by the Pages preparation command."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'docs' / 'tools' / 'prepare_versioned_site.py'
VERSION = '2.0.0-beta.1'


class VersionedDocsTests(unittest.TestCase):
    def test_candidate_has_versioned_urls_and_locked_source_links(self):
        revision = subprocess.check_output(
            ['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'candidate docs'
            command = [sys.executable, SCRIPT, VERSION, revision, output]
            subprocess.run(command, check=True, timeout=30)
            metadata = json.loads((output / 'candidate.json').read_text())
            self.assertEqual(metadata, {
                'version': VERSION, 'revision': revision, 'path': f'{VERSION}/',
            })
            site = output / VERSION
            for page in ('', 'api', 'getting-started', 'sdk'):
                html = (site / page / 'index.html').read_text()
                self.assertIn(f'https://jepemo.github.io/malvado/{VERSION}/', html)
                self.assertIn(f'https://github.com/jepemo/malvado/blob/{revision}/', html)
                for route in ('blob', 'tree', 'edit'):
                    self.assertNotIn(f'github.com/jepemo/malvado/{route}/master/', html)
            self.assertIn('https://jepemo.github.io/malvado/legacy/',
                          (site / 'index.html').read_text())
            self.assertIn(f'git checkout --detach {revision}',
                          (site / 'getting-started' / 'index.html').read_text())

            repeat = subprocess.run(command, capture_output=True, timeout=30)
            self.assertNotEqual(repeat.returncode, 0)
            self.assertTrue((site / 'index.html').is_file())

            invalid = output.parent / 'invalid'
            wrong = subprocess.run([sys.executable, SCRIPT, VERSION, 'HEAD', invalid],
                                   capture_output=True, timeout=30)
            self.assertNotEqual(wrong.returncode, 0)
            self.assertFalse(invalid.exists())


if __name__ == '__main__':
    unittest.main()
