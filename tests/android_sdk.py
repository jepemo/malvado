#!/usr/bin/env python3
"""Test isolated SDK publication and cleanup without downloading Android tools."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
import json
from pathlib import Path
import sys
import tempfile
import subprocess
import unittest
from unittest.mock import patch
import zipfile

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_sdk
import test_android_window as window


class SdkTests(unittest.TestCase):
    def package(self, *, complete=True):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as bundle:
            bundle.writestr('ndk/source.properties', 'Pkg.Revision=27.2.12479018\n')
            if complete:
                bundle.writestr('ndk/toolchain.cmake', 'toolchain')
        return data.getvalue()

    def install(self, output, data, *, digest=None):
        packages = [('test.zip', digest or hashlib.sha256(data).hexdigest(), 'ndk', 'ndk/version', 'toolchain.cmake', False)]
        with patch.object(android_sdk, 'PACKAGES', packages), patch.object(android_sdk.urllib.request, 'urlopen', return_value=io.BytesIO(data)):
            android_sdk.prepare(output)

    def test_install_records_digest_and_preserves_unrelated_files(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'keep').write_text('unrelated')
            data = self.package()
            self.install(root / 'SDK with spaces', data)
            sdk = root / 'SDK with spaces'
            self.assertEqual((sdk / 'ndk/version/toolchain.cmake').read_text(), 'toolchain')
            self.assertEqual(json.loads((sdk / 'malvado-archives.json').read_text()),
                             [dict(archive='test.zip', sha256=hashlib.sha256(data).hexdigest(), destination='ndk/version')])
            self.assertEqual({p.name for p in root.iterdir()}, {'keep', 'SDK with spaces'})

    def test_failure_cleans_staging_without_publishing_sdk(self):
        for complete, digest in [(True, '0' * 64), (False, None)]:
            with self.subTest(complete=complete), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                (root / 'keep').write_text('unrelated')
                with self.assertRaises(ValueError):
                    self.install(root / 'sdk', self.package(complete=complete), digest=digest)
                self.assertEqual([p.name for p in root.iterdir()], ['keep'])

    def test_download_failure_and_existing_destinations(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with patch.object(android_sdk.urllib.request, 'urlopen', side_effect=OSError('offline')):
                with self.assertRaises(OSError):
                    android_sdk.prepare(root / 'sdk')
            self.assertEqual(list(root.iterdir()), [])
            (root / 'sdk').mkdir()
            (root / 'sdk/keep').write_text('pre-existing')
            with self.assertRaises(ValueError):
                android_sdk.prepare(root / 'sdk')
            self.assertEqual((root / 'sdk/keep').read_text(), 'pre-existing')

    def test_later_package_failure_removes_earlier_staged_packages(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = self.package()
            package = ('first.zip', hashlib.sha256(data).hexdigest(), 'ndk', 'ndk/version', 'toolchain.cmake', False)
            packages = [package, ('second.zip', '0' * 64, *package[2:])]
            with patch.object(android_sdk, 'PACKAGES', packages), patch.object(android_sdk.urllib.request, 'urlopen', side_effect=lambda *a, **kw: io.BytesIO(data)):
                with self.assertRaisesRegex(ValueError, 'SHA-256'):
                    android_sdk.prepare(root / 'sdk')
            self.assertEqual(list(root.iterdir()), [])

    def test_publication_failure_removes_only_claimed_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / 'sdk'
            (root / 'keep').write_text('unrelated')
            move = android_sdk.shutil.move
            def fail_publication(source, destination):
                if Path(destination).parent == output:
                    move(source, destination)
                    raise OSError('publication failed')
                return move(source, destination)
            with patch.object(android_sdk.shutil, 'move', side_effect=fail_publication):
                with self.assertRaisesRegex(OSError, 'publication failed'):
                    self.install(output, self.package())
            self.assertEqual([p.name for p in root.iterdir()], ['keep'])

    def packaging_sdk(self, root):
        tools = root / 'build-tools/34.0.0'
        tools.mkdir(parents=True)
        for name in ('aapt', 'zipalign', 'apksigner'):
            (tools / name).touch()
        (tools / 'source.properties').write_text('Pkg.Revision=34.0.0\n')
        platform = root / 'platforms/android-34'
        platform.mkdir(parents=True)
        (platform / 'android.jar').touch()
        (platform / 'source.properties').write_text('Pkg.Revision=3\nAndroidVersion.ApiLevel=34\nAndroidVersion.ExtensionLevel=7\n')
        return tools, platform

    def test_packaging_facts_identify_actual_platform_and_java(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            tools, _ = self.packaging_sdk(root)
            facts = {}
            with patch.object(window, 'run', return_value=subprocess.CompletedProcess([], 0, 'openjdk version "21.0.7" 2025-04-15 LTS\nOpenJDK Runtime Environment Temurin-21.0.7+6 (build 21.0.7+6-LTS)')):
                self.assertEqual(window.packaging_tools(root, io.StringIO(), facts), tools)
            self.assertEqual(facts, dict(build_tools='34.0.0', platform_revision='3', platform_extension='7', java='openjdk version "21.0.7" 2025-04-15 LTS\nOpenJDK Runtime Environment Temurin-21.0.7+6 (build 21.0.7+6-LTS)'))

    def test_java_accepts_pinned_vendor_version_and_build_on_both_newlines(self):
        output = 'openjdk version "21.0.7" 2025-04-15 LTS\nOpenJDK Runtime Environment Temurin-21.0.7+6 (build 21.0.7+6-LTS)'
        window.verify_java(output)
        window.verify_java(output.replace('\n', '\r\n'))

    def test_java_rejects_wrong_patch_build_vendor_or_missing_metadata(self):
        valid = 'openjdk version "21.0.7" 2025-04-15 LTS\nOpenJDK Runtime Environment Temurin-21.0.7+6 (build 21.0.7+6-LTS)'
        for output in ('', valid.splitlines()[0], valid.replace('21.0.7', '21.0.8'), valid.replace('+6', '+7'), valid.replace('Temurin', 'OtherJDK')):
            with self.subTest(output=output), self.assertRaisesRegex(RuntimeError, 'Temurin 21.0.7'):
                window.verify_java(output)

    def test_packaging_rejects_mislabeled_tools_and_wrong_api(self):
        for invalid in ('tools', 'api', 'jar'):
            with self.subTest(invalid=invalid), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                tools, platform = self.packaging_sdk(root)
                if invalid == 'tools':
                    (tools / 'source.properties').write_text('Pkg.Revision=35.0.0\n')
                elif invalid == 'api':
                    (platform / 'source.properties').write_text('AndroidVersion.ApiLevel=35\n')
                else:
                    (platform / 'android.jar').unlink()
                with self.assertRaises(RuntimeError):
                    window.packaging_tools(root, io.StringIO(), {})


if __name__ == '__main__':
    unittest.main()
