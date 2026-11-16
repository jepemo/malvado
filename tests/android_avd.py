#!/usr/bin/env python3
"""Check reproducible AVD metadata and exclusive ownership."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import sys
import tempfile
import unittest
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from android_avd import create


class AvdTests(unittest.TestCase):
    def image(self, root, abi='x86_64'):
        image = root / 'image with spaces'
        image.mkdir()
        (image / 'source.properties').write_text(
            f'AndroidVersion.ApiLevel=34\nSystemImage.Abi={abi}\nSystemImage.TagId=google_apis\n')
        for name in ('system.img', 'userdata.img', 'ramdisk.img'):
            (image / name).touch()
        return image

    def test_both_abis_absolute_image_and_owned_destination(self):
        for abi, arch in [('x86_64', 'x86_64'), ('arm64-v8a', 'arm64')]:
            with self.subTest(abi=abi), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                image = self.image(root, abi)
                output = root / 'avd with spaces'
                create(output, image)
                config = (output / 'malvado-alpha8.avd/config.ini').read_text()
                self.assertIn(f'image.sysdir.1={image.resolve()}\n', config)
                self.assertIn(f'hw.cpu.arch={arch}\n', config)
                self.assertIn(f'path={output.resolve()}/malvado-alpha8.avd\n', (output / 'malvado-alpha8.ini').read_text())
                before = config
                with self.assertRaises(FileExistsError):
                    create(output, image)
                self.assertEqual((output / 'malvado-alpha8.avd/config.ini').read_text(), before)
                self.assertFalse(list(root.glob('.malvado-avd-*')))

    def test_invalid_or_incomplete_images_do_not_publish(self):
        for invalid in ('api', 'abi', 'tag', 'missing'):
            with self.subTest(invalid=invalid), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                image = self.image(root)
                properties = image / 'source.properties'
                text = properties.read_text()
                if invalid == 'missing':
                    (image / 'system.img').unlink()
                else:
                    properties.write_text(text.replace({'api': '34', 'abi': 'x86_64', 'tag': 'google_apis'}[invalid], 'invalid'))
                with self.assertRaises(ValueError):
                    create(root / 'avd', image)
                self.assertFalse((root / 'avd').exists())
                self.assertFalse(list(root.glob('.malvado-avd-*')))


if __name__ == '__main__':
    unittest.main()
