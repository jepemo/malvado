#!/usr/bin/env python3
"""Verify project selection, runtime identity and exclusive APK publication."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import sys
import os
import tempfile
import unittest
import shutil
import subprocess
import zipfile
import xml.etree.ElementTree as ET
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import android_export as exporter


class ExportTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.project = self.root / 'game with spaces'
        self.project.mkdir()
        (self.project / 'main.lua').write_text('return {}')

    def test_manifest_configuration_and_xml_escaping(self):
        xml = exporter.manifest_xml('org.example.game', version_code=42, version_name='2 & "next"', title='Juego <uno>', orientation='portrait', min_api=30)
        root = ET.fromstring(xml)
        namespace = '{http://schemas.android.com/apk/res/android}'
        self.assertEqual(root.get(namespace+'versionCode'), '42')
        self.assertEqual(root.get(namespace+'versionName'), '2 & "next"')
        self.assertEqual(root.find('application').get(namespace+'label'), 'Juego <uno>')
        self.assertEqual(root.find('.//activity').get(namespace+'screenOrientation'), 'portrait')
        self.assertEqual(root.find('uses-sdk').get(namespace+'minSdkVersion'), '30')

    def test_invalid_manifest_inputs_fail_before_tools_or_output(self):
        for configuration in (dict(version_code=0), dict(version_code=2100000001), dict(version_code=True),
                              dict(min_api=27), dict(min_api=35), dict(orientation='sideways'),
                              dict(title=''), dict(title='x'*129), dict(title='bad\nline'),
                              dict(version_name='bad\x00value')):
            output = self.root / 'never' / 'game.apk'
            with self.subTest(configuration=configuration), patch.object(exporter, 'run') as run:
                with self.assertRaises(ValueError):
                    exporter.export(self.project, self.root, self.root, output, 'org.game', 'test', **configuration)
                run.assert_not_called()
            self.assertFalse(output.parent.exists())

    def test_nested_binary_assets_and_source_selection(self):
        (self.project / 'assets').mkdir()
        (self.project / 'assets/value.bin').write_bytes(b'\0\r\n\xff')
        (self.project / '.hidden').write_text('secret')
        (self.project / 'dist').mkdir()
        (self.project / 'dist/old.apk').write_bytes(b'old')
        files = exporter.project_files(self.project)
        self.assertEqual({name for _, name in files}, {'main.lua', 'assets/value.bin'})

    def test_links_reserved_paths_and_newlines_rejected(self):
        for name in (('licenses', 'malvado-files.txt') if os.name == 'nt' else ('licenses', 'malvado-files.txt', 'line\nbreak', 'back\\slash')):
            path = self.project / name
            path.write_text('bad')
            with self.subTest(name=name), self.assertRaises(ValueError):
                exporter.project_files(self.project)
            path.unlink()
        try:
            (self.project / 'link').symlink_to(self.project / 'main.lua')
        except OSError:
            if os.name == 'nt': return
            raise
        with self.assertRaisesRegex(ValueError, 'links'):
            exporter.project_files(self.project)

    def test_missing_entry_is_explicit(self):
        (self.project / 'main.lua').unlink()
        with self.assertRaisesRegex(ValueError, 'main.lua'):
            exporter.project_files(self.project)

    def test_runtime_version_system_and_elf_identity(self):
        runtime = self.root / 'sdk'
        metadata = runtime / 'share/malvado/malvado-sdk-info.txt'
        metadata.parent.mkdir(parents=True)
        library = runtime / 'libexec/malvado/android/arm64-v8a/libgame.so'
        library.parent.mkdir(parents=True)
        header = bytearray(20)
        header[:7] = b'\x7fELF\x02\x01\x01'
        header[16:18] = b'\x03\x00'
        header[18:20] = b'\xb7\x00'
        library.write_bytes(header)
        metadata.write_text('version=test\nsystem=android\narchitecture=aarch64\n')
        self.assertEqual(exporter.validate_runtime(runtime, 'test'), library)
        with self.assertRaisesRegex(ValueError, 'mismatch'):
            exporter.validate_runtime(runtime, 'wrong')
        for metadata_text in ('version=test\nsystem=linux\narchitecture=aarch64\n',
                              'version=test\nsystem=android\narchitecture=x86_64\n'):
            metadata.write_text(metadata_text)
            with self.assertRaisesRegex(ValueError, 'mismatch'):
                exporter.validate_runtime(runtime, 'test')
        metadata.write_text('version=test\nsystem=android\narchitecture=aarch64\n')
        library.write_bytes(b'not a runtime')
        with self.assertRaisesRegex(ValueError, 'ELF'):
            exporter.validate_runtime(runtime, 'test')

    def test_failed_signing_cleans_staging_without_publishing(self):
        runtime = self.root / 'runtime'
        library = runtime / 'libgame.so'
        runtime.mkdir()
        library.write_bytes(b'runtime')
        notices = runtime / 'share/licenses/malvado'
        notices.mkdir(parents=True)
        for name in ('LICENSE', 'Lua-readme.html', 'raylib-LICENSE', 'cJSON-LICENSE'):
            (notices / name).write_text('notice')
        sdk = self.root / 'sdk'
        for name in ('build-tools/34.0.0/aapt', 'build-tools/34.0.0/zipalign', 'build-tools/34.0.0/apksigner', 'platforms/android-34/android.jar'):
            path = sdk / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.touch()
        (sdk / 'build-tools/34.0.0/source.properties').write_text('Pkg.Revision=34.0.0\n')
        (sdk / 'platforms/android-34/source.properties').write_text('AndroidVersion.ApiLevel=34\n')
        def run(args):
            if str(args[0]).endswith('/aapt'):
                with zipfile.ZipFile(args[-1], 'w'): pass
            elif str(args[0]).endswith('/zipalign'):
                shutil.copyfile(args[-2], args[-1])
            else:
                raise subprocess.CalledProcessError(1, args)
        output = self.root / 'game.apk'
        with patch.object(exporter, 'validate_runtime', return_value=library), patch.object(exporter.shutil, 'which', return_value='/tool'), patch.object(exporter, 'run', side_effect=run):
            with self.assertRaises(subprocess.CalledProcessError):
                exporter.export(self.project, runtime, sdk, output, 'org.game', 'test')
        self.assertFalse(output.exists())
        self.assertFalse(list(self.root.glob('.malvado-apk-*')))
        (sdk / 'build-tools/34.0.0/source.properties').write_text('Pkg.Revision=wrong\n')
        with patch.object(exporter, 'validate_runtime', return_value=library), patch.object(exporter, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'version metadata'):
                exporter.export(self.project, runtime, sdk, output, 'org.game', 'test')
        run.assert_not_called()


    def test_existing_output_and_invalid_id_do_not_run_tools(self):
        output = self.root / 'game.apk'
        output.write_bytes(b'keep')
        with patch.object(exporter, 'run') as run:
            with self.assertRaises(ValueError):
                exporter.export(self.project, self.root, self.root, output, 'org.game', 'test')
            with self.assertRaises(ValueError):
                exporter.export(self.project, self.root, self.root, self.root / 'other.apk', '../bad', 'test')
        run.assert_not_called()
        self.assertEqual(output.read_bytes(), b'keep')


if __name__ == '__main__':
    unittest.main()
