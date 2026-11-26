#!/usr/bin/env python3
"""Run the Lua gameplay recipes from fresh directories and verify their output."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile


SOURCE = Path(__file__).resolve().parents[1] / 'examples' / 'gameplay-recipes'
RECIPES = {
    'game-states': (
        'menu', 'play', 'Time left: 1', 'pause', 'play', 'Time left: 0', 'game over',
    ),
    'finite-state-machine': ('Patrol', 'Chase from 2', 'Rest at 4', 'Guard recovered'),
    'spawn-enemies-and-bullets': (
        'Bullet reached the right edge', 'Enemy reached the left edge', 'Wave clear',
    ),
    'coordinate-processes': ('Open request queued', 'Door opened', 'Exit ready'),
    'level-lifecycle': ('Level 1 cleared', 'HUD score: 10', 'Level 2 cleared', 'HUD score: 20'),
}


def verify(cli, downloads):
    for name, expected in RECIPES.items():
        filename = name + '.lua'
        source = (downloads / f'recipe-{name}' if downloads else SOURCE) / filename
        with tempfile.TemporaryDirectory(prefix='malvado-recipe-') as directory:
            shutil.copyfile(source, Path(directory) / filename)
            result = subprocess.run(
                [cli, 'run', '--headless', filename], cwd=directory,
                text=True, capture_output=True, timeout=5,
            )
            if result.returncode or result.stdout.splitlines() != list(expected):
                raise RuntimeError(f'{filename} failed ({result.returncode}):\n'
                                   f'{result.stdout}{result.stderr}\nExpected: {expected}')
            print(f'PASS: {filename}', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cli', type=Path)
    parser.add_argument('--downloads', type=Path)
    args = parser.parse_args()
    verify(args.cli.resolve(), args.downloads)
