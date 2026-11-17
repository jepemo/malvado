#!/usr/bin/env python3
"""Collect verified desktop SDK archives after the exact-revision platform gate."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
from pathlib import Path
import shutil
from sdk_artifact import inspect_archive


def collect(desktop, windows, report, output):
    evidence = json.loads(Path(report).read_text())
    if len(evidence['runs']) != 3 or {run['workflow'] for run in evidence['runs']} != {'ci.yml', 'windows.yml', 'android.yml'} or any(
            run['sha'] != evidence['sha'] or run['conclusion'] != 'success' for run in evidence['runs']):
        raise ValueError('Complete same-revision platform evidence is required')
    artifacts = []
    for root, expected in ((Path(desktop), {('darwin', 'arm64'), ('linux', 'x86_64')}),
                           (Path(windows), {('windows', 'AMD64')})):
        archives = list(root.glob('*.tar.gz'))
        if len(archives) != len(expected):
            raise ValueError('Unexpected SDK archive count')
        seen = set()
        for archive in archives:
            facts = inspect_archive(archive)
            target = facts['system'], facts['architecture']
            if target not in expected or target in seen:
                raise ValueError('Unexpected or duplicate SDK target')
            seen.add(target)
            artifacts.append((archive, facts))
    if len({facts['version'] for _, facts in artifacts}) != 1:
        raise ValueError('SDK versions do not match')
    output = Path(output)
    output.mkdir()  # An existing candidate belongs to the caller.
    try:
        for archive, _ in artifacts:
            shutil.copyfile(archive, output / archive.name)
            shutil.copyfile(str(archive) + '.sha256', output / (archive.name + '.sha256'))
        evidence['sdks'] = [facts for _, facts in artifacts]
        (output / 'platform-candidate.json').write_text(json.dumps(evidence, indent=2) + '\n')
    except BaseException:
        shutil.rmtree(output)
        raise
    return evidence


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('desktop', 'windows', 'report', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    collect(args.desktop, args.windows, args.report, args.output)


if __name__ == '__main__':
    main()
