#!/usr/bin/env python3
"""Verify the three-language lessons with an installed desktop SDK."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time


EXAMPLES = Path(__file__).resolve().parents[1] / 'examples' / 'features'
TRACES = {
    'startup': ['ready'],
    'processes': ['first: 1', 'second: 1', 'first: 2', 'second: 2'],
    'image': ['image: 32x32'],
}
LANGUAGES = {'c': ['--c'], 'lua': [], 'rust': ['--lang', 'rust']}
ENTRY_POINTS = {'c': 'main.c', 'lua': 'main.lua', 'rust': 'main.rs'}


def run(command, cwd, env=None):
    result = subprocess.run(command, cwd=cwd, check=False, text=True,
                            capture_output=True, timeout=120, env=env)
    if result.returncode:
        raise RuntimeError(f'{command} failed\n{result.stdout}{result.stderr}')
    return result.stdout


def check_trace(output, expected):
    prefixes = tuple(line.split(':')[0] for line in expected)
    trace = [line for line in output.splitlines() if line.startswith(prefixes)]
    if trace != expected:
        raise RuntimeError(f'Expected {expected}, got {trace}\n{output}')


def check_pixels(player, cwd, env):
    # The lesson exits on its own. Capture both the tile and the background
    # while it is visible, then require a normal exit.
    with subprocess.Popen([player], cwd=cwd, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True, env=env) as game:
        try:
            deadline = time.monotonic() + 8
            while game.poll() is None and time.monotonic() < deadline:
                query = subprocess.run(
                    ['xdotool', 'search', '--onlyvisible', '--name', '^Malvado Image$'],
                    capture_output=True, text=True, timeout=2,
                )
                windows = query.stdout.split()
                if len(windows) == 1:
                    pixels = subprocess.run([
                        'import', '-window', windows[0], '-crop', '48x48+0+0',
                        '-depth', '8', 'rgb:-',
                    ], capture_output=True, timeout=2)
                    tile = (40 * 48 + 40) * 3
                    background = (8 * 48 + 8) * 3
                    if (pixels.returncode == 0 and
                            pixels.stdout[tile:tile + 3] == bytes((255, 0, 0)) and
                            pixels.stdout[background:background + 3] == bytes(3)):
                        break
                time.sleep(0.05)
            else:
                if game.poll() is not None:
                    output, _ = game.communicate()
                    raise RuntimeError(f'Image lesson exited before capture ({game.returncode})\n{output}')
                raise RuntimeError('The exported image lesson did not show the red tile')
            output, _ = game.communicate(timeout=10)
            if game.returncode:
                raise RuntimeError(f'Exported image lesson failed\n{output}')
            return output
        finally:
            if game.poll() is None:
                game.kill()


def verify(sdk, work, pixels):
    cli = sdk / 'bin' / 'malvado'
    player_env = dict(os.environ, PATH=str(work / 'no-toolchain'))
    for topic, trace in TRACES.items():
        for language, options in LANGUAGES.items():
            lesson = EXAMPLES / topic
            project = work / f'{topic}-{language}'
            run([cli, 'new', *options, project], work)
            entry = ENTRY_POINTS[language]
            shutil.copyfile(lesson / language / entry, project / entry)
            asset = lesson / 'assets' / 'tile.png'
            if asset.is_file():
                shutil.copyfile(asset, project / asset.name)
            args = ['--headless'] if language == 'lua' and topic != 'image' else []
            check_trace(run([cli, 'run', *args], project), trace)
            exported = work / f'{topic}-{language}-export'
            run([cli, 'export', exported], project)
            if asset.is_file() and (exported / asset.name).read_bytes() != asset.read_bytes():
                raise RuntimeError('The exported tile differs from the shared asset')
            if not (exported / 'licenses' / 'LICENSE').is_file():
                raise RuntimeError('The export is missing the project license')
            player = exported / 'game'
            output = check_pixels(player, work, player_env) if pixels and topic == 'image' else run(
                [player, *args], work, player_env)
            check_trace(output, trace)
            print(f'PASS: {topic} ({language}) from installed SDK and exported folder', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--pixels', action='store_true', help='check pixels on Linux X11')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='malvado beta examples ') as directory:
        verify(args.sdk.resolve(), Path(directory), args.pixels)


if __name__ == '__main__':
    main()
