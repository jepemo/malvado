#!/usr/bin/env python3
"""Run a Cargo test or SDK probe in an owned Android emulator directory."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import sys
import subprocess
import uuid
import time

sys.dont_write_bytecode = True
from test_android import run, shell


def create_remote(prefix, log, remote):
    # Android can reject shell creation under transient fork memory pressure.
    # Retry only directory preparation. Never replay a test or ABI probe.
    deadline = time.monotonic() + 30
    while (remaining := deadline - time.monotonic()) > 0:
        try:
            shell(prefix, log, ['mkdir', '-p', remote], timeout=min(10, remaining))
            return
        except RuntimeError as error:
            if 'Android command transport failed: error: fork failed: Out of memory' not in str(error):
                raise
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            log.write('Retrying Android directory preparation after fork memory pressure\n')
            log.flush()
            time.sleep(min(1, remaining))
    raise RuntimeError('Android directory preparation exceeded its 30-second fork-memory deadline')


def execute(adb, serial, program, arguments, log, *, inputs=None):
    if not serial.startswith('emulator-') or not serial[9:].isdigit():
        raise ValueError('Select an explicit emulator serial')
    program = Path(program).resolve(strict=True)
    prefix = [str(adb), '-s', serial]
    remote = '/data/local/tmp/malvado-rust-' + uuid.uuid4().hex
    create_remote(prefix, log, remote)
    try:
        run(log, prefix + ['push', program, remote + '/program'], timeout=30)
        environment = ['TMPDIR=' + remote]
        if inputs is not None:
            inputs = Path(inputs).resolve(strict=True)
            if not inputs.is_dir() or any(path.is_symlink() for path in inputs.rglob('*')):
                raise ValueError('Test inputs must be a directory without symlinks')
            run(log, prefix + ['push', inputs, remote + '/inputs'], timeout=30)
            environment.append('MALVADO_TEST_ROOT=' + remote + '/inputs')
        result = shell(prefix, log, [remote + '/program', *arguments], cwd=remote,
                       environment=environment, timeout=180, check=False)
        return result
    finally:
        shell(prefix, log, ['rm', '-rf', remote], timeout=30)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('program')
    parser.add_argument('arguments', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    try:
        # Keep transport logs off stdout: ABI probes parse stdout as integers.
        result = execute(os.environ['MALVADO_ANDROID_ADB'], os.environ['MALVADO_ANDROID_SERIAL'],
                         args.program, args.arguments, sys.stderr, inputs=os.environ.get('MALVADO_ANDROID_TEST_INPUTS'))
        sys.stdout.write(result.stdout)
        return result.returncode
    except (OSError, RuntimeError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f'Android Rust transport failed: {error}', file=sys.stderr)
        return 125


if __name__ == '__main__':
    sys.exit(main())
