#!/usr/bin/env python3
"""Check pinned AAB tooling integrity and base-module assembly."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import sys
import os
import tempfile
import unittest
import subprocess
from unittest.mock import patch
import zipfile
sys.dont_write_bytecode = True
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import android_bundle as bundle
import android_export as exporter
import android_bundle_test as verifier


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
            dex=work/'classes.dex'; dex.write_bytes(b'DEX')
            calls=[]
            def run(args):
                calls.append(args)
                if 'link' in args:
                    with zipfile.ZipFile(work/'protobuf.zip','w') as archive:
                        archive.writestr('AndroidManifest.xml',b'protobuf')
                        archive.writestr('resources.pb',b'table')
            with patch.object(bundle,'validate_tools',return_value=(work/'bundletool.jar',work/'aapt2')):
                bundle.build(run,work,work,work,work/'manifest',library,[(data,'nested/value.bin')],dex)
            with zipfile.ZipFile(work/'base.zip') as archive:
                self.assertEqual(archive.read('manifest/AndroidManifest.xml'),b'protobuf')
                self.assertEqual(archive.read('dex/classes.dex'),b'DEX')
                self.assertEqual(archive.read('assets/nested/value.bin'),b'\0\xff')
                self.assertEqual(archive.read('lib/arm64-v8a/libgame.so'),b'ELF')
                self.assertEqual(archive.read('assets/malvado-files.txt'),b'nested/value.bin\n')
            self.assertIn('validate',calls[-1])

    def test_signing_uses_environment_references_and_rejects_unsigned_success(self):
        signing=(Path('/external/key.p12'),'alias','STORE_SECRET','KEY_SECRET')
        calls=[]
        with patch.object(bundle.subprocess,'run',return_value=subprocess.CompletedProcess([],0,'jar verified.')) as verify:
            bundle.sign(lambda args:calls.append(args),Path('game.aab'),signing)
        self.assertIn('-storepass:env',calls[0])
        self.assertIn('STORE_SECRET',calls[0])
        self.assertIn('-keypass:env',calls[0])
        self.assertIn('-strict',verify.call_args.args[0])
        with patch.object(bundle.subprocess,'run',return_value=subprocess.CompletedProcess([],0,'jar is unsigned.')):
            with self.assertRaisesRegex(ValueError,'not verified'):
                bundle.sign(lambda args:None,Path('game.aab'),signing)

    @unittest.skipIf(os.name == 'nt', 'AAB authoring requires Unix file permissions')
    def test_derived_apk_password_file_is_private_and_removed(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            calls=[]
            def run(args, **kwargs):
                calls.append(args)
                if 'build-apks' in args:
                    password=Path(next(arg.removeprefix('--ks-pass=file:') for arg in args if arg.startswith('--ks-pass=file:')))
                    self.assertEqual(password.read_text(),'test-secret')
                    self.assertEqual(password.stat().st_mode & 0o777,0o600)
                    output=Path(next(arg.removeprefix('--output=') for arg in args if arg.startswith('--output=')))
                    with zipfile.ZipFile(output,'w') as archive: archive.writestr('universal.apk',b'APK')
                return subprocess.CompletedProcess(args,0,b'')
            with patch.object(verifier,'validate_tools',return_value=(root/'bundletool.jar',root/'aapt2')), patch.object(verifier.subprocess,'run',side_effect=run):
                verifier.derive_apk(root/'game.aab',root/'game.apk',root,root/'key','alias','PASSWORD',environment={'PASSWORD':'test-secret'})
            self.assertEqual((root/'game.apk').read_bytes(),b'APK')
            self.assertFalse(list(root.glob('.bundle-apks-*')))
            self.assertNotIn('test-secret',str(calls))

    def test_aab_requires_release_tools_and_matching_output_before_publication(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            for options in (dict(output_format='aab'),dict(output_format='aab',release=True)):
                with self.assertRaises(ValueError): exporter.export(root,root,root,root/'new/game.aab','org.game','test',**options)
                self.assertFalse((root/'new').exists())


if __name__=='__main__': unittest.main()
