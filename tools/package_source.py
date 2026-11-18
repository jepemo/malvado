#!/usr/bin/env python3
"""Package the tracked source of one Malvado commit."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[1]


def package_source(revision, output):
    git = ['git', '-C', str(ROOT)]
    sha = subprocess.check_output(
        [*git, 'rev-parse', '--verify', '--end-of-options', f'{revision}^{{commit}}'],
        text=True,
    ).strip()
    prefix = f'malvado-source-{sha}/'
    output.mkdir(parents=True)
    archive = output / f'{prefix[:-1]}.tar.gz'
    try:
        subprocess.run([
            *git, 'archive', '--format=tar.gz', f'--prefix={prefix}',
            f'--add-virtual-file={prefix}SOURCE-REVISION:{sha}\n',
            f'--output={archive.resolve()}', sha,
        ], check=True)
        with archive.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        archive.with_name(archive.name + '.sha256').write_bytes(
            f'{digest}  {archive.name}\n'.encode('ascii')
        )
    except BaseException:
        shutil.rmtree(output)
        raise
    return archive


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('revision', help='Git revision to package')
    parser.add_argument('output', type=Path, help='new directory for the archive and checksum')
    args = parser.parse_args()
    try:
        print(package_source(args.revision, args.output))
    except (OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
