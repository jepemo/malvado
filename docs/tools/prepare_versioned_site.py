#!/usr/bin/env python3
"""Build a versioned MkDocs snapshot from a committed Malvado revision."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import io
import json
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import tempfile

from mkdocs.commands.build import build
from mkdocs.config import load_config


ROOT = Path(__file__).resolve().parents[2]
REPOSITORY = 'https://github.com/jepemo/malvado'
PAGES = 'https://jepemo.github.io/malvado'
BRANCH_LINK = re.compile(rf'({re.escape(REPOSITORY)}/(?:blob|tree)/)master/')


def prepare(version, revision, output):
    if not re.fullmatch(r'2\.\d+\.\d+(?:-(?:alpha|beta|rc)\.\d+)?', version):
        raise ValueError('expected a Malvado 2 version such as 2.0.0-beta.1')
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('expected a full lowercase Git commit ID')
    snapshot = subprocess.check_output([
        'git', '-C', str(ROOT), 'archive', revision, '--',
        'docs/mkdocs.yml', 'docs/site', f'docs/versions/{version}.md',
    ])
    with tempfile.TemporaryDirectory(prefix='malvado-docs-') as temporary:
        staged = Path(temporary)
        with tarfile.open(fileobj=io.BytesIO(snapshot)) as files:
            if files.pax_headers.get('comment') != revision:
                raise ValueError('Git archive does not identify the requested commit')
            files.extractall(staged, filter='data')
        docs = staged / 'docs'
        for page in (docs / 'site').rglob('*.md'):
            original = page.read_text(encoding='utf-8')
            page.write_text(BRANCH_LINK.sub(rf'\g<1>{revision}/', original), encoding='utf-8')

        start = docs / 'site' / 'getting-started.md'
        heading, _, content = start.read_text(encoding='utf-8').partition('\n')
        start.write_text(
            f'{heading}\n\n'
            f'This documentation snapshot for {version} uses commit `{revision}`. '
            f'When building from Git, run `git checkout --detach {revision}` '
            f'after entering the cloned directory and before running `mise install`.\n'
            f'{content}', encoding='utf-8',
        )
        output.mkdir(parents=True)
        try:
            config = load_config(
                config_file=str(docs / 'mkdocs.yml'), strict=True,
                site_dir=str((output / version).resolve()),
                site_url=f'{PAGES}/{version}/', site_name=f'Malvado {version}',
                edit_uri='',
            )
            build(config)
            facts = {'version': version, 'revision': revision, 'path': f'{version}/'}
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
    args = parser.parse_args()
    try:
        prepare(args.version, args.revision, args.output)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
