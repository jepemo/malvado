#!/usr/bin/env python3
"""Verify the examples actually published in the current documentation build."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile
import time
from zipfile import ZipFile

from dodge_tutorial import CHECKPOINTS, verify as verify_dodge
from gameplay_recipes import RECIPES, verify as verify_recipes

WORKERS = (r'first: 1', r'second: 1', r'first: 2', r'second: 2')
HEADLESS = {
    'two-workers': WORKERS,
    'two-workers-c': WORKERS,
    'two-workers-rust': WORKERS,
    'roll': (r'Dice roll: [1-6]',),
    'level-teardown': (r'Level complete', r'Level: returned', r'Enemy: dead',
                       r'Music: running', r'Music: killed'),
    'movement-time': (r'60 fps: 12\.00 px after 0\.10 s', r'30 fps: 12\.00 px after 0\.10 s'),
    'mailbox': (r'Score: 10', r'Score: 15', r'Final score: 15'),
    'reference-cameras': (r'Camera moved to 15, 15', r'Camera followed to 70, 40'),
    'reference-collision': (r'Rectangle tests passed', r'Shape tests passed', r'Process collision found'),
    'reference-fades': (r'Overlay: 0 0 0 0', r'Reached opaque blue', r'Overlay: 0 0 0 0', r'Fade queue finished'),
    'reference-fpg-metadata': (r'Sprite 7: 2x2, .+', r'Sprite 42: 2x1, .+'),
    'reference-import-globals': (r'Malvado globals installed',),
    'reference-process-control': (r'Found two workers, one attached', r'Level and workers stopped'),
    'reference-random-seed': (r'Repeated seed gave the same roll:\s+[1-6]',),
    'reference-startup': (r'Initial size: 320x240', r'New size: 640x360',
                          r'Process cadence: 30', r'Process cadence: 20',
                          r'Elapsed wall time: \d+\.\d+ seconds'),
    'reference-world': (r'First contact at 92 pixels', r'Player moved 92 pixels and stopped at the wall'),
}
# A trace is an ordered sequence of full lines. Raylib writes its own log lines.
WINDOW_TRACES = {
    'image': (r'image: 32x32',),
    'image-c': (r'image: 32x32',),
    'image-rust': (r'image: 32x32',),
    'reference-images': (r'Loaded tile: 32x32',),
    'reference-fpg-image': (r'Sprite image: 2x2',),
    'reference-text': (r'Score label: [1-9]\d* × [1-9]\d* pixels',),
    'reference-audio': (r'Sound voice stopped', r'Music stream stopped'),
    'reference-atlas': (r'First sprite: one, \d+x\d+', r'Animation finished'),
    'reference-input': (r'Space pressed', r'Space released'),
}
# These programs close on their own after showing the result.
FINITE_WINDOWS = {
    'reference-title', 'reference-graphics', 'reference-shapes', 'reference-legacy-sheet',
    'reference-images', 'reference-fpg-image', 'reference-text', 'reference-audio', 'reference-atlas',
    'image', 'image-c', 'image-rust',
}
INTERACTIVE_WINDOWS = {
    'aseprite-bounce', 'audio-tone', 'blinking-text', 'camera-follow', 'catch-lua', 'catch-c',
    'catch-rust', 'circle-collision', 'hello', 'input-movement', 'meteors', 'reference-input',
    'select-sprites', 'shape-coordinates', 'space-asteroids', 'text-labels', 'world-wall',
}
WINDOWED = FINITE_WINDOWS | INTERACTIVE_WINDOWS
RED = b'\xff\x00\x00'
CYAN = b'\x00\xff\xff'
ORANGE = b'\xff\xa5\x00'
EXPECTED_COLORS = {
    **dict.fromkeys(('image', 'image-c', 'image-rust', 'reference-images', 'reference-legacy-sheet'), (RED,)),
    **dict.fromkeys(('catch-lua', 'catch-c', 'catch-rust'), (CYAN, ORANGE)),
    'meteors': (ORANGE,),
    'reference-shapes': (ORANGE,),
    'shape-coordinates': (ORANGE,),
    'world-wall': (bytes((80, 190, 255)), bytes((255, 165, 70))),
}


def run(command, directory):
    result = subprocess.run(command, cwd=directory, text=True, capture_output=True, timeout=120)
    if result.returncode:
        raise RuntimeError(f'{command} failed in {directory}\n{result.stdout}{result.stderr}')
    return result.stdout


def check_trace(output, expected):
    lines = iter(output.splitlines())
    for pattern in expected:
        if not any(re.fullmatch(pattern, line) for line in lines):
            raise ValueError(f'Missing expected line {pattern!r} in order:\n{output}')


def extract_example(download, work):
    if download.suffix == '.zip':
        with ZipFile(download) as archive:
            archive.extractall(work)
    else:
        shutil.copyfile(download, work / download.name)
    entries = [path for path in work.iterdir() if path.suffix in ('.lua', '.c', '.rs')]
    if len(entries) != 1:
        raise ValueError(f'Expected one entry script or source file in {download}')
    return entries[0]


def prepare(cli, download, work):
    entry = extract_example(download, work)
    if entry.suffix == '.lua':
        run([cli, 'check', entry.name], work)
        return work, [entry.name], True

    language = 'c' if entry.suffix == '.c' else 'rust'
    project = work / 'lesson'
    run([cli, 'new', '--lang', language, project.name], work)
    shutil.copyfile(entry, project / f'main{entry.suffix}')
    for asset in work.iterdir():
        if asset.is_dir() and asset != project:
            shutil.copytree(asset, project / asset.name)
        elif asset.is_file() and asset != entry and asset.name not in ('LICENSE', 'NOTICE'):
            shutil.copyfile(asset, project / asset.name)
    # Keep compiler time outside the window startup deadline.
    run([cli, 'build'], project)
    return project, [], False


def windows():
    result = subprocess.run(['xdotool', 'search', '--onlyvisible', '--name', '.'],
                            text=True, capture_output=True, timeout=3)
    if result.returncode not in (0, 1) or result.stderr:
        raise RuntimeError(f'Cannot list X11 windows: {result.stderr}')
    return set(result.stdout.splitlines())


def rendered(window, lesson):
    result = subprocess.run(['import', '-window', window, '-colorspace', 'sRGB', '-depth', '8', 'rgb:-'],
                            capture_output=True, timeout=3)
    if result.returncode or len(result.stdout) < 6:
        return False
    data = memoryview(result.stdout)
    if colors := EXPECTED_COLORS.get(lesson):
        return all(any(data[index:index + 3] == color for index in range(0, len(data), 3)) for color in colors)
    background = data[:3]
    return any(data[index:index + 3] != background for index in range(3, len(data), 3))


def verify_window(command, directory, lesson):
    existing = windows()
    # A native CLI run can launch a child. Kill its process group on timeout.
    with tempfile.TemporaryFile(mode='w+') as log, subprocess.Popen(
            command, cwd=directory, stdout=log, stderr=subprocess.STDOUT, start_new_session=True) as game:
        try:
            if lesson != 'reference-audio':
                deadline = time.monotonic() + 15
                while game.poll() is None and time.monotonic() < deadline:
                    visible = windows() - existing
                    if len(visible) == 1:
                        window = next(iter(visible))
                        title_matches = (lesson != 'reference-title' or
                                         run(['xdotool', 'getwindowname', window], directory).strip() == 'Level 2')
                        if title_matches and rendered(window, lesson):
                            break
                    time.sleep(0.03)
                else:
                    raise RuntimeError('Expected game window and rendered content did not appear')
                if lesson == 'reference-input':
                    run(['xdotool', 'windowfocus', '--sync', window, 'keydown', 'space',
                         'sleep', '0.2', 'keyup', 'space', 'sleep', '0.2'], directory)
                if lesson in INTERACTIVE_WINDOWS:
                    run(['wmctrl', '-ic', f'{int(window):#x}'], directory)
            if game.wait(timeout=12):
                raise RuntimeError(f'Game exited with status {game.returncode}')
            log.seek(0)
            check_trace(log.read(), WINDOW_TRACES.get(lesson, ()))
        except (OSError, RuntimeError, ValueError, subprocess.TimeoutExpired) as error:
            log.seek(0)
            raise RuntimeError(f'{lesson}: {error}\n{log.read()}') from error
        finally:
            if game.poll() is None:
                os.killpg(game.pid, signal.SIGKILL)


def verify(cli, downloads, headless, include):
    tutorial = {f'dodge-{step}' for step in CHECKPOINTS}
    recipes = {f'recipe-{name}' for name in RECIPES}
    expected = HEADLESS.keys() | WINDOWED | tutorial | recipes
    published = {path.name for path in downloads.iterdir()}
    if published != expected:
        raise ValueError(f'Example coverage differs. Missing downloads: {sorted(expected - published)}. '
                         f'Downloads without checks: {sorted(published - expected)}.')
    selected = set(include) if include else expected
    if selected - expected:
        raise ValueError(f'Unknown example ID: {sorted(selected - expected)}')
    if headless:
        if include and selected & WINDOWED:
            raise ValueError(f'Examples require a display: {sorted(selected & WINDOWED)}')
        if not include:
            print('Headless subset: desktop examples and native examples require the full SDK/X11 check.')
            selected -= WINDOWED | {'two-workers-c', 'two-workers-rust'}
    if selected & tutorial:
        verify_dodge(cli, downloads, windowed=not headless)
    if selected & recipes:
        verify_recipes(cli, downloads)
    for lesson in sorted(selected - tutorial - recipes):
        download, = sorted((downloads / lesson).iterdir())
        with tempfile.TemporaryDirectory(prefix=f'malvado-doc-{lesson}-') as temporary:
            directory, args, lua = prepare(cli, download, Path(temporary))
            if lesson in HEADLESS:
                check_trace(run([cli, 'run', *(['--headless'] if lua else []), *args], directory),
                            HEADLESS[lesson])
            else:
                verify_window([cli, 'run', *args], directory, lesson)
        print(f'PASS: published example {lesson}', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cli', type=Path)
    parser.add_argument('downloads', type=Path)
    parser.add_argument('--headless', action='store_true', help='run the Lua examples that support headless mode')
    parser.add_argument('examples', nargs='*', help='optional example IDs for focused checks')
    args = parser.parse_intermixed_args()
    verify(args.cli.resolve(), args.downloads.resolve(), args.headless, args.examples)
