#!/usr/bin/env python3
"""Check release note validation, publication visibility and rendering parity."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from release_notes import NOTES, candidate, index, publications, public_notes

SHA = 'a' * 40
BODY = '''## Highlights

A release with [../api/game.md#play](../api/game.md#play) and a screenshot.
![Game](example.png)

## Breaking changes

See [Highlights](#highlights).

```text
[Leave fenced examples alone](does-not-exist.md)
```
'''


def entry(version, date='2026-11-26', status='stable'):
    return {'version': version, 'published': date, 'status': status, 'docs': 'site'}


class ReleaseNotesTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.version = '2.2.0'
        self.write('docs/api/game.md', '# Play\n')
        self.write(f'{NOTES}/example.png', 'image fixture')
        self.write(f'{NOTES}/unreleased.md', '# Unreleased\n\nNext change.\n')
        self.write(f'{NOTES}/{self.version}.md', f'# Malvado {self.version}\n\n{BODY}')
        self.set_publications([])

    def write(self, path, text):
        destination = self.root / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(text, encoding='utf-8')

    def read(self, path):
        return (self.root / path).read_bytes()

    def set_publications(self, entries):
        self.write(f'{NOTES}/publications.json', json.dumps(entries))

    def test_stable_and_prerelease_render_from_same_notes(self):
        for version, status in (('2.2.0', 'stable'), ('2.3.0-rc.1', 'prerelease')):
            with self.subTest(version=version):
                self.write(f'{NOTES}/{version}.md', f'# Malvado {version}\n\n{BODY}')
                self.set_publications([])
                release = candidate(self.read, version, SHA)
                self.assertEqual(release, candidate(self.read, version, SHA))
                self.assertEqual(public_notes(self.root), [])
                self.set_publications([entry(version, status=status)])
                [page] = public_notes(self.root)
                self.assertEqual(release.split('\n\n', 1)[1].strip().replace(SHA, f'v{version}'), page['body'])
                self.assertIn(f'[../api/game.md#play](https://github.com/jepemo/malvado/blob/{SHA}/docs/api/game.md#play)', release)
                self.assertIn(f'/raw/{SHA}/{NOTES}/example.png', release)
                self.assertNotIn('Next change', index([page]))
                self.assertIn('Breaking changes', index([page]))
                self.assertIn(status.title(), index([page]))
                with self.assertRaisesRegex(ValueError, 'already published'):
                    candidate(self.read, version, SHA)

    def test_rejects_invalid_links(self):
        for replacement, message in (
            ('../api/missing.md', 'missing.md'),
            ('../api/game.md#missing', 'Missing Markdown anchor'),
            ('#missing', 'Missing release note anchor'),
            ('../../../elsewhere.md', 'escapes the repository'),
        ):
            with self.subTest(replacement=replacement):
                self.write(f'{NOTES}/{self.version}.md',
                           f'# Malvado {self.version}\n\n' + BODY.replace('../api/game.md#play', replacement))
                with self.assertRaisesRegex((ValueError, FileNotFoundError), message):
                    candidate(self.read, self.version, SHA)

    def test_rejects_unfinalized_notes(self):
        for text in ('## Highlights\n\n## Fixed\n\n- Something\n',
                     '## Highlights\n\nTODO',
                     '## Highlights\n\n  '):
            with self.subTest(text=text):
                self.write(f'{NOTES}/{self.version}.md', f'# Malvado {self.version}\n\n{text}')
                with self.assertRaisesRegex(ValueError, 'completed Highlights'):
                    candidate(self.read, self.version, SHA)

    def test_rejects_invalid_publication_metadata(self):
        for entries in ([entry('2.3.0-alpha.1')], [entry('2.3.0', '2026-02-30')],
                        [entry('2.3.0'), entry('2.3.0')],
                        [entry('2.2.0', '2026-10-01'), entry('2.3.0')],
                        [entry('unreleased')]):
            with self.subTest(entries=entries):
                self.set_publications(entries)
                with self.assertRaises(ValueError):
                    publications(self.read)


if __name__ == '__main__':
    unittest.main()
