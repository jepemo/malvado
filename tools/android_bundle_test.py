#!/usr/bin/env python3
"""Shared verifier support for executing APKs derived from app bundles."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import subprocess
import tempfile
import zipfile
from android_bundle import validate_tools


def derive_apk(bundle, output, tools, key, alias, password_env, environment=None):
    jar, _ = validate_tools(Path(tools))
    with tempfile.TemporaryDirectory(prefix='.bundle-apks-', dir=output.parent) as folder:
        work = Path(folder)
        password = work/'password'
        # bundletool accepts file references, not jarsigner's env references.
        with open(password, 'x', opener=lambda path, flags: os.open(path, flags, 0o600)) as stream:
            stream.write((environment if environment is not None else os.environ)[password_env])
        apks = work/'game.apks'
        subprocess.run(['java', '-jar', str(jar), 'validate', '--bundle='+str(bundle)], check=True, capture_output=True, timeout=120)
        subprocess.run(['java', '-jar', str(jar), 'build-apks', '--bundle='+str(bundle), '--output='+str(apks),
                        '--mode=universal', '--ks='+str(key), '--ks-key-alias='+alias,
                        '--ks-pass=file:'+str(password)], check=True, capture_output=True, timeout=120)
        with zipfile.ZipFile(apks) as archive:
            output.write_bytes(archive.read('universal.apk'))
    return output
