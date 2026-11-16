#!/usr/bin/env python3
"""Wait for a selected Android emulator with a bounded boot deadline."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import math
import subprocess
import time


def wait(adb, serial, timeout=240):
    if not serial.startswith('emulator-') or not serial[9:].isdigit():
        raise ValueError('Select an explicit emulator serial')
    if not math.isfinite(timeout) or timeout <= 0:
        raise ValueError('Boot timeout must be positive and finite')
    deadline = time.monotonic() + timeout
    last = 'no response'
    while (remaining := deadline - time.monotonic()) > 0:
        try:
            result = subprocess.run(
                [str(adb), '-s', serial, 'shell', 'getprop', 'sys.boot_completed'],
                capture_output=True, text=True, timeout=min(10, remaining))
            if result.returncode == 0 and result.stdout.strip() == '1':
                return
            last = (result.stderr or result.stdout).strip()[-1000:]
        except subprocess.TimeoutExpired:
            last = 'adb command timed out'
        remaining = deadline - time.monotonic()
        if remaining > 0:
            time.sleep(min(2, remaining))
    raise RuntimeError(f'{serial} did not boot within {timeout:g} seconds: {last}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--adb', required=True)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--timeout', type=float, default=240)
    args = parser.parse_args()
    wait(args.adb, args.serial, args.timeout)


if __name__ == '__main__':
    main()
