#!/usr/bin/env python3
"""Verify that Cargo revalidates native SDK probes after toolchain changes."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import subprocess
import os



def verify(log, manifest, environment, *, release=False):
    # CMake applies the other generator environment defaults only when this
    # selector is explicit. Keep the caller's selected Windows toolchain.
    environment = dict(environment)
    environment.setdefault('CMAKE_GENERATOR', 'Visual Studio 17 2022' if os.name == 'nt' else 'Unix Makefiles')
    command = ['cargo', 'check', '--offline', '--manifest-path', str(manifest)]
    if release:
        command.append('--release')

    def check(env, label):
        result = subprocess.run(command, env=env, text=True, capture_output=True, timeout=180)
        log.write(f'\n>>> Rust toolchain refresh: {label}\n{result.stdout}{result.stderr}')
        log.flush()
        return result

    result = check(environment, 'baseline')
    if result.returncode:
        raise RuntimeError('Rust toolchain baseline failed: ' + (result.stdout + result.stderr)[-4000:])
    for name in ('CMAKE_GENERATOR_PLATFORM', 'CMAKE_GENERATOR_TOOLSET', 'CMAKE_GENERATOR_INSTANCE'):
        invalid = 'malvado-invalid-selection'
        result = check(dict(environment, **{name: invalid}), name)
        output = result.stdout + result.stderr
        if result.returncode == 0 or invalid not in output:
            raise RuntimeError(f'Cargo did not revalidate {name}: {output[-4000:]}')
        result = check(environment, 'restore ' + name)
        if result.returncode:
            raise RuntimeError(f'Rust native probe did not recover after {name}: ' + (result.stdout + result.stderr)[-4000:])
        log.write(f'PASS: {name} change rejected; restored selection accepted\n')
        log.flush()
