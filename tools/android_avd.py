#!/usr/bin/env python3
"""Create an isolated API 34 verification AVD without SDK Manager or Java."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from pathlib import Path
import shutil
import tempfile


def create(output, image):
    output = Path(output).resolve()
    image = Path(image).resolve()
    if any(c in str(image) for c in '\n\r'):
        raise ValueError('Image path cannot contain newlines')
    properties = {}
    for line in (image / 'source.properties').read_text().splitlines():
        if '=' in line:
            key, value = line.split('=', 1)
            properties[key.strip()] = value.strip()
    abi = properties.get('SystemImage.Abi')
    if properties.get('AndroidVersion.ApiLevel') != '34' or abi not in ('x86_64', 'arm64-v8a') or properties.get('SystemImage.TagId') != 'google_apis':
        raise ValueError('Expected API 34 Google APIs arm64-v8a or x86_64 image')
    for name in ('system.img', 'userdata.img', 'ramdisk.img'):
        if not (image / name).is_file():
            raise ValueError('Incomplete Android image: ' + name)
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix='.malvado-avd-', dir=output.parent))
    try:
        avd = staging / 'malvado-alpha8.avd'
        avd.mkdir()
        config = {
            'AvdId': 'malvado-alpha8', 'avd.ini.encoding': 'UTF-8',
            'abi.type': abi, 'hw.cpu.arch': 'arm64' if abi == 'arm64-v8a' else 'x86_64',
            'image.sysdir.1': str(image), 'tag.id': 'google_apis',
            'hw.lcd.width': '1080', 'hw.lcd.height': '1920', 'hw.lcd.density': '420',
            'hw.ramSize': '2048', 'hw.cpu.ncore': '2', 'hw.keyboard': 'yes',
            'hw.gpu.enabled': 'yes', 'hw.gpu.mode': 'swiftshader_indirect',
            'disk.dataPartition.size': '2G', 'showDeviceFrame': 'no',
        }
        (avd / 'config.ini').write_text(''.join(f'{key}={value}\n' for key, value in config.items()))
        (staging / 'malvado-alpha8.ini').write_text(
            f'avd.ini.encoding=UTF-8\npath={output / avd.name}\ntarget=android-34\n')
        # An exclusive destination keeps cleanup limited to this invocation.
        output.mkdir()
        try:
            for item in staging.iterdir():
                item.rename(output / item.name)
        except BaseException:
            shutil.rmtree(output)
            raise
    finally:
        shutil.rmtree(staging)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--image', required=True, type=Path)
    args = parser.parse_args()
    create(args.output, args.image)


if __name__ == '__main__':
    main()
