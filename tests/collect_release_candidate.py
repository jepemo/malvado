#!/usr/bin/env python3
"""Exercise the draft bundle gate with exact-revision fixture artifacts."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from collect_release_candidate import collect
from sdk_artifact import inspect_archive


SHA = 'a' * 40
VERSION = '2.0.0-beta.1'
NOTICE = b'expected dependency notice\n'
TARGETS = [('darwin', 'arm64'), ('linux', 'x86_64'), ('windows', 'AMD64')]


def package(path, files, revision=None):
    path.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(path, 'w:gz', pax_headers={'comment': revision} if revision else None) as archive:
        for name, content in files.items():
            member = tarfile.TarInfo(name)
            member.size = len(content)
            archive.addfile(member, io.BytesIO(content))
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    path.with_name(path.name + '.sha256').write_bytes(f'{digest}  {path.name}\n'.encode())
    return digest


class DraftCandidateTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        self.platform, self.source, self.docs = (root / name for name in ('platform', 'source', 'docs'))
        self.output, self.notes = root / 'output', root / 'notes.md'
        self.notes.write_text(f'# Malvado {VERSION}\n')
        self.write_platform()
        prefix = f'malvado-source-{SHA}'
        archive = self.source / 'package' / f'{prefix}.tar.gz'
        digest = package(archive, {
            f'{prefix}/SOURCE-REVISION': f'{SHA}\n'.encode(),
            f'{prefix}/licenses/raylib-NOTICES.txt': NOTICE,
        }, SHA)
        self.source_report = {
            'revision': SHA, 'archive': archive.name, 'sha256': digest,
            'core_tests': 'passed', 'installed_c_sdk': 'passed',
            'verified_notices': ['raylib-NOTICES.txt'],
        }
        self.write_source_report()
        (self.docs / VERSION).mkdir(parents=True)
        (self.docs / VERSION / 'index.html').write_text('<title>Candidate</title>')
        self.write_docs(SHA)

    def write_platform(self, replacement=NOTICE):
        sdks = []
        for system, arch in TARGETS:
            archive = self.platform / f'malvado-sdk-{VERSION}-{system}-{arch}.tar.gz'
            package(archive, {
                'share/malvado/malvado-sdk-info.txt':
                    f'version={VERSION}\nsystem={system}\narchitecture={arch}\n'.encode(),
                'share/licenses/malvado/raylib-NOTICES.txt':
                    replacement if system == 'darwin' else NOTICE,
            })
            sdks.append(inspect_archive(archive))
        report = {'sha': SHA, 'sdks': sdks, 'runs': [
            dict(workflow=name, sha=SHA, conclusion='success')
            for name in ('ci.yml', 'windows.yml', 'android.yml')
        ]}
        (self.platform / 'platform-candidate.json').write_text(json.dumps(report))

    def write_docs(self, revision):
        (self.docs / 'candidate.json').write_text(json.dumps(
            dict(version=VERSION, revision=revision, path=f'{VERSION}/')))

    def write_source_report(self):
        (self.source / 'verification.json').write_text(json.dumps(self.source_report))

    def collect(self):
        return collect(self.platform, self.source, self.docs, self.notes, VERSION, self.output)

    def test_collects_artifacts_and_checksums(self):
        self.collect()
        self.assertEqual(len(list(self.output.glob('*.tar.gz'))), 4)
        checks = (self.output / 'SHA256SUMS').read_text().splitlines()
        self.assertEqual(len(checks), 13)
        for entry in checks:
            digest, name = entry.split('  ', 1)
            self.assertEqual(digest, hashlib.sha256((self.output / name).read_bytes()).hexdigest())
        with self.assertRaises(FileExistsError):
            self.collect()
        self.assertEqual((self.output / 'RELEASE-NOTES.md').read_bytes(), self.notes.read_bytes())

    def test_version_page_releases_only_its_release_notes(self):
        self.notes.write_text(f'# {VERSION}\n\nStatus: planned.\n\n'
                              '## Release notes\n\n### Changes\n\n- Delivered.\n\n'
                              '## Checklist\n\n- [ ] Verify release.\n')
        self.collect()
        self.assertEqual((self.output / 'RELEASE-NOTES.md').read_text(),
                         f'# Malvado {VERSION}\n\n### Changes\n\n- Delivered.\n')

    def test_version_page_requires_matching_nonempty_release_notes(self):
        for heading, section in ((VERSION, '## Checklist\n\nPending.'),
                                  (VERSION, '## Release notes\n\n## Checklist\n\nPending.'),
                                  ('2.0.0', '## Release notes\n\n- Delivered.')):
            with self.subTest(heading=heading, section=section):
                self.notes.write_text(f'# {heading}\n\n{section}\n')
                with self.assertRaisesRegex(ValueError, 'Release notes'):
                    self.collect()
                self.assertFalse(self.output.exists())

    def test_wrong_documentation_or_source_evidence_is_rejected(self):
        self.write_docs('b' * 40)
        with self.assertRaisesRegex(ValueError, 'Documentation'):
            self.collect()
        self.write_docs(SHA)
        self.source_report['core_tests'] = 'failed'
        self.write_source_report()
        with self.assertRaisesRegex(ValueError, 'Source verification'):
            self.collect()
        self.assertFalse(self.output.exists())

    def test_changed_sdk_notice_is_rejected_even_with_a_valid_checksum(self):
        self.write_platform(replacement=b'other notice')
        with self.assertRaisesRegex(ValueError, 'dependency notices'):
            self.collect()
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
