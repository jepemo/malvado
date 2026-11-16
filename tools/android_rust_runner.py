#!/usr/bin/env python3
"""Run a Cargo test or SDK probe in an owned Android emulator directory."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import sys
import subprocess
import uuid

sys.dont_write_bytecode = True
from test_android import run, shell


def execute(adb, serial, program, arguments, log):
    if not serial.startswith('emulator-') or not serial[9:].isdigit():
        raise ValueError('Select an explicit emulator serial')
    program = Path(program).resolve(strict=True)
    prefix = [str(adb), '-s', serial]
    remote = '/data/local/tmp/malvado-rust-' + uuid.uuid4().hex
    shell(prefix, log, ['mkdir', '-p', remote])
    try:
        run(log, prefix + ['push', program, remote + '/program'], timeout=30)
        result = shell(prefix, log, [remote + '/program', *arguments], cwd=remote,
                       environment=['TMPDIR=' + remote], timeout=180, check=False)
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
                         args.program, args.arguments, sys.stderr)
        sys.stdout.write(result.stdout)
        return result.returncode
    except (OSError, RuntimeError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f'Android Rust transport failed: {error}', file=sys.stderr)
        return 125


if __name__ == '__main__':
    sys.exit(main())
