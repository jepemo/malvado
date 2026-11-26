#!/usr/bin/env python3
"""Build a versioned documentation snapshot from a committed Malvado revision."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[2]
WEBSITE = ROOT / 'docs' / 'website'
sys.path.insert(0, str(ROOT / 'tools'))
from release_notes import REVISION, VERSION, candidate, read_tree


def prepare(version, revision, output, require_notes=False):
    if not re.fullmatch(VERSION, version):
        raise ValueError('expected a Malvado 2 version such as 2.0.0-beta.1')
    if not re.fullmatch(REVISION, revision):
        raise ValueError('expected a full lowercase Git commit ID')
    if output.exists():
        raise FileExistsError(f'output directory already exists: {output}')
    notes = None
    if require_notes:
        notes = candidate(lambda path: read_tree(ROOT, path, revision), version, revision).encode('utf-8')
    sources = [
        'docs/site/getting-started.md', 'docs/site/sdk.md', 'docs/site/api.md',
        'docs/api',
        'examples/catch/lua/main.lua', 'examples/catch/c/main.c',
        'bindings/rust/malvado/examples/catch.rs',
        'examples/helloworld/main.lua',
        'examples/meteors/main.lua',
        'examples/move/main.lua',
        'LICENSE',
    ]
    git = ['git', '-C', str(ROOT)]
    optional_lessons = [
        'examples/rust', 'examples/rand', 'examples/features/image',
        'docs/site/learn', 'examples/features/processes', 'examples/process-lesson',
        'examples/applied-learn', 'examples/aseprite', 'examples/audio',
        'examples/camera-follow', 'examples/collision', 'examples/blinking-text',
        'examples/world-collision', 'examples/space_asteroids',
        'docs/site/tutorials', 'examples/dodge',
        'docs/site/patterns', 'examples/gameplay-recipes', 'examples/lua-reference',
        'examples/fpg/graphics.fpg', 'tests/fixtures/fonts/AnonymousPro-Bold.ttf',
        'tests/fixtures/fonts/OFL.txt',
    ]
    for path in optional_lessons:
        if subprocess.check_output([*git, 'ls-tree', revision, '--', path], text=True):
            sources.append(path)
    snapshot = subprocess.check_output([*git, 'archive', revision, '--', *sources])
    with tempfile.TemporaryDirectory(prefix='malvado-docs-') as temporary:
        staged = Path(temporary)
        with tarfile.open(fileobj=io.BytesIO(snapshot)) as files:
            if files.pax_headers.get('comment') != revision:
                raise ValueError('Git archive does not identify the requested commit')
            files.extractall(staged, filter='data')
        website = staged / 'docs' / 'website'
        website.mkdir()
        for filename in ('astro.config.mjs', 'package.json', 'package-lock.json'):
            shutil.copyfile(WEBSITE / filename, website / filename)
        for directory in ('public', 'scripts', 'src'):
            shutil.copytree(WEBSITE / directory, website / directory,
                            ignore=shutil.ignore_patterns('example-downloads'))
        env = dict(os.environ, MALVADO_DOCS_VERSION=version, MALVADO_DOCS_REVISION=revision)
        env.pop('MALVADO_DOCS_RELEASE_NOTES', None)
        facts = {'version': version, 'revision': revision, 'path': f'{version}/'}
        if notes is not None:
            notes_path = staged / 'RELEASE-NOTES.md'
            notes_path.write_bytes(notes)
            env['MALVADO_DOCS_RELEASE_NOTES'] = str(notes_path)
            facts['release_notes_sha256'] = hashlib.sha256(notes).hexdigest()
        output.mkdir(parents=True)
        try:
            subprocess.run(['npm', 'ci', '--no-audit', '--no-fund'], cwd=website, check=True)
            subprocess.run(
                ['npm', 'run', 'build', '--', '--outDir', str((output / version).resolve())],
                cwd=website, env=env, check=True,
            )
            (output / 'candidate.json').write_text(
                json.dumps(facts, indent=2) + '\n', encoding='utf-8')
        except BaseException:
            shutil.rmtree(output)
            raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('version')
    parser.add_argument('revision')
    parser.add_argument('output', type=Path, help='new directory for the snapshot')
    parser.add_argument('--require-notes', action='store_true', help='require finalized canonical release notes')
    args = parser.parse_args()
    try:
        prepare(args.version, args.revision, args.output, args.require_notes)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
