#!/usr/bin/env python3
"""Check pinned AAB tooling integrity and base-module assembly."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import hashlib
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile
sys.dont_write_bytecode = True
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import android_bundle as bundle
import android_export as exporter


class BundleTests(unittest.TestCase):
    def test_unsupported_host_does_not_create_output(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'tools'
            with self.assertRaises(ValueError): bundle.install(output,'unknown')
            self.assertFalse(output.exists())

    def test_existing_tools_and_failed_download_are_preserved_or_cleaned(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'tools'
            output.mkdir()
            (output/'keep').write_text('keep')
            with self.assertRaises(FileExistsError): bundle.install(output,'linux')
            self.assertEqual((output/'keep').read_text(),'keep')
            (output/'keep').unlink()
            output.rmdir()
            with patch.object(bundle.urllib.request,'urlopen',side_effect=OSError('offline')):
                with self.assertRaises(OSError): bundle.install(output,'linux')
            self.assertFalse(output.exists())

    def test_corrupt_archives_and_modified_executable_fail(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            (root/'bundletool.jar').write_bytes(b'corrupt')
            with self.assertRaises(ValueError): bundle.validate_tools(root,'linux')
            (root/'aapt2.jar').write_bytes(b'also corrupt')
            with patch.object(bundle,'BUNDLETOOL_SHA256',bundle.digest(root/'bundletool.jar')):
                with self.assertRaises(ValueError): bundle.validate_tools(root,'linux')
            with zipfile.ZipFile(root/'aapt2.jar','w') as archive: archive.writestr('aapt2',b'expected')
            (root/'aapt2').write_bytes(b'modified')
            with patch.object(bundle,'BUNDLETOOL_SHA256',bundle.digest(root/'bundletool.jar')), patch.dict(bundle.AAPT2,{'linux':('linux',bundle.digest(root/'aapt2.jar'))}):
                with self.assertRaisesRegex(ValueError,'executable'): bundle.validate_tools(root,'linux')

    def test_base_module_contains_native_assets_and_protobuf_manifest(self):
        with tempfile.TemporaryDirectory() as folder:
            work=Path(folder)
            library=work/'libgame.so'; library.write_bytes(b'ELF')
            data=work/'asset'; data.write_bytes(b'\0\xff')
            calls=[]
            def run(args):
                calls.append(args)
                if 'link' in args:
                    with zipfile.ZipFile(work/'protobuf.zip','w') as archive:
                        archive.writestr('AndroidManifest.xml',b'protobuf')
                        archive.writestr('resources.pb',b'table')
            with patch.object(bundle,'validate_tools',return_value=(work/'bundletool.jar',work/'aapt2')):
                bundle.build(run,work,work,work,work/'manifest',library,[(data,'nested/value.bin')])
            with zipfile.ZipFile(work/'base.zip') as archive:
                self.assertEqual(archive.read('manifest/AndroidManifest.xml'),b'protobuf')
                self.assertEqual(archive.read('assets/nested/value.bin'),b'\0\xff')
                self.assertEqual(archive.read('lib/arm64-v8a/libgame.so'),b'ELF')
                self.assertEqual(archive.read('assets/malvado-files.txt'),b'nested/value.bin\n')
            self.assertIn('validate',calls[-1])

    def test_aab_requires_release_tools_and_matching_output_before_publication(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            for options in (dict(output_format='aab'),dict(output_format='aab',release=True)):
                with self.assertRaises(ValueError): exporter.export(root,root,root,root/'new/game.aab','org.game','test',**options)
                self.assertFalse((root/'new').exists())


if __name__=='__main__': unittest.main()
