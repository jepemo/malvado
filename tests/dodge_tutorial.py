#!/usr/bin/env python3
"""Run every Dodge! checkpoint in a fresh Lua project."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
CHECKPOINTS = (
    '01-start', '02-player', '03-movement', '04-enemy',
    '05-spawn', '06-collision', '07-score', 'main',
)


def run(command, directory, env=None):
    result = subprocess.run(command, cwd=directory, env=env, text=True,
                            capture_output=True, timeout=45, check=False)
    if result.returncode:
        raise RuntimeError(f'{command} failed in {directory}:\n{result.stdout}{result.stderr}')
    return result.stdout


def verify(cli, downloads, windowed):
    with tempfile.TemporaryDirectory(prefix='malvado-dodge-') as directory:
        work = Path(directory)
        for step in CHECKPOINTS:
            project = work / step
            run([cli, 'new', step], work)
            source = (downloads / f'dodge-{step}' / f'{step}.lua' if downloads else
                      HERE.parent / 'examples' / 'dodge' / f'{step}.lua')
            shutil.copyfile(source, project / 'main.lua')
            run([cli, 'check'], project)

            harness = project / 'verify.lua'
            shutil.copyfile(HERE / 'dodge_tutorial.lua', harness)
            options = [] if windowed else ['--headless']
            env = dict(os.environ, MALVADO_DODGE_CHECKPOINT=step)
            output = run([cli, 'run', *options, harness.name], project, env)
            assert f'PASS: Dodge! {step}' in output, output
            print(f'PASS: Dodge! {step}', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cli', type=Path)
    parser.add_argument('--downloads', type=Path)
    parser.add_argument('--windowed', action='store_true')
    args = parser.parse_args()
    verify(args.cli.resolve(), args.downloads, args.windowed)
