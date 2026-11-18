#!/usr/bin/env python3
"""Run real installed-CLI Lua APK exports on an explicit arm64 emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from android_export import GAME_ACTIVITY
import os
import json
from pathlib import Path
import subprocess
import tempfile
import shutil
import time
import uuid
import zipfile
import zlib
import struct


def center_pixel(png):
    offset, data = 8, b''
    while offset < len(png):
        length = int.from_bytes(png[offset:offset+4], 'big')
        kind = png[offset+4:offset+8]
        value = png[offset+8:offset+8+length]
        if kind == b'IHDR':
            width, height, depth, color, _, _, interlace = struct.unpack('>IIBBBBB', value)
            if depth != 8 or color not in (2, 6) or interlace:
                raise ValueError('Unsupported screenshot PNG')
        if kind == b'IDAT': data += value
        offset += 12 + length
    raw = zlib.decompress(data)
    bpp = 4 if color == 6 else 3
    stride = width * bpp
    previous = bytearray(stride)
    for y in range(height // 2 + 1):
        start = y * (stride + 1)
        mode = raw[start]
        row = bytearray(raw[start+1:start+1+stride])
        for x in range(stride):
            a = row[x-bpp] if x >= bpp else 0
            b = previous[x]
            c = previous[x-bpp] if x >= bpp else 0
            if mode == 1: row[x] = (row[x] + a) & 255
            elif mode == 2: row[x] = (row[x] + b) & 255
            elif mode == 3: row[x] = (row[x] + (a+b)//2) & 255
            elif mode == 4:
                p = a+b-c
                distance = (abs(p-a), abs(p-b), abs(p-c))
                predictor = (a,b,c)[distance.index(min(distance))]
                row[x] = (row[x] + predictor) & 255
            elif mode != 0: raise ValueError('Invalid screenshot filter')
        previous = row
    index = width // 2 * bpp
    return tuple(previous[index:index+3])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cli', 'runtime-sdk', 'android-sdk', 'work'):
        parser.add_argument('--'+name, required=True, type=Path)
    runner = parser.add_mutually_exclusive_group(required=True)
    runner.add_argument('--serial')
    runner.add_argument('--package-only', action='store_true',
                        help='Verify production packages on the authoring host without an emulator')
    parser.add_argument('--release', action='store_true')
    parser.add_argument('--text-input', action='store_true')
    parser.add_argument('--format', choices=('apk','aab'), default='apk')
    parser.add_argument('--bundle-tools', type=Path)
    args = parser.parse_args()
    if args.package_only and args.text_input:
        parser.error('Text input verification requires an emulator serial')
    if args.format == 'aab' and (not args.release or not args.bundle_tools):
        parser.error('AAB checks require --release and --bundle-tools')
    args.work.mkdir(parents=True, exist_ok=True)
    (args.work / 'results.json').unlink(missing_ok=True)
    for mode in ('explicit', 'declarative', 'error'):
        for suffix in ('.png', '-export.log', '-unsigned-export.log', '-runtime.log', '-manifest.log'):
            (args.work / (mode + suffix)).unlink(missing_ok=True)
    cli = args.cli.resolve()
    adb = [str(args.android_sdk.resolve()/'platform-tools/adb'), '-s', args.serial]
    def command(values, **kwargs):
        return subprocess.run([str(value) for value in values], check=True, capture_output=True, timeout=120, **kwargs)

    def export(values, log, **kwargs):
        try:
            result = command(values, **kwargs)
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
            log.write_bytes((error.stdout or b'') + (error.stderr or b''))
            raise
        log.write_bytes(result.stdout + result.stderr)
    if not args.package_only:
        command(adb+['shell','input','keyevent','224'])
        command(adb+['shell','wm','dismiss-keyguard'])
    with tempfile.TemporaryDirectory(prefix='malvado-test-signing-', dir=args.work.resolve()) as key_folder:
        signing_arguments = []
        signing_environment = dict(os.environ)
        if args.release:
            key = Path(key_folder) / 'test-only.p12'
            command(['keytool', '-genkeypair', '-keystore', key, '-storepass', 'android', '-keypass', 'android',
                     '-alias', 'test', '-keyalg', 'RSA', '-validity', '30', '-dname', 'CN=Malvado release test only'])
            signing_environment['MALVADO_TEST_STORE_PASSWORD'] = 'android'
            signing_arguments = ['--release', '--keystore', key, '--key-alias', 'test', '--store-password-env', 'MALVADO_TEST_STORE_PASSWORD', '--key-password-env', 'MALVADO_TEST_STORE_PASSWORD']
        results=[]
        for mode in (('explicit','declarative') if args.text_input else ('explicit','declarative','error')):
            with tempfile.TemporaryDirectory(prefix='game with spaces ',dir=args.work.resolve()) as folder:
                project=Path(folder)
                (project/'assets').mkdir()
                for name in ('atlas.json', 'red.png'):
                    shutil.copyfile(Path(__file__).resolve().parents[1]/'tests/fixtures'/name,project/name)
                (project/'assets/value.bin').write_bytes(b'\0\r\n\xff')
                (project/'behavior.lua').write_text('''local mv = require('malvado')
    local task = mv.process(function()
     local f = assert(io.open('assets/value.bin','rb'))
     assert(f:read('a') == string.char(0,13,10,255))
     f:close()
     local atlas = mv.load_asset('atlas.json')
     assert(mv.sprite_id(atlas, 'red') == 7)
     local image = mv.load_image('red.png')
     local w,h = mv.image_size(image)
     assert(w == 8 and h == 8)
     if MODE_ERROR then error('expected exported process error') end
     local text = ''
     for i=1,(MODE_TEXT and 900 or 360) do
      if MODE_TEXT then
       while true do
        local character = mv.text_input_next()
        if not character then break end
        text = text .. character
       end
       assert(('aZ0'):sub(1,#text) == text)
       if text == 'aZ0' then
        io.stderr:write('exported Lua text input passed\\n')
        return
       end
      end
      mv.set_color(255,0,0)
      mv.draw_box(0,0,320,240)
      mv.frame()
     end
     assert(not MODE_TEXT, 'text input timeout')
    end)
    return function() task() end
    ''')
                entry="return {width=320,height=240,init=require('behavior')}" if mode=='declarative' else "require('malvado').start(require('behavior'),{width=320,height=240})"
                if mode=='error': entry='MODE_ERROR=true\n'+entry
                if args.text_input: entry='MODE_TEXT=true\n'+entry
                (project/'main.lua').write_text(entry)
                package='dev.malvado.exported.p'+uuid.uuid4().hex
                apk=project.parent/(package+'.'+args.format)
                arguments=[cli,'export','--target','android-arm64','--format',args.format,'--runtime-sdk',args.runtime_sdk.resolve(),'--android-sdk',args.android_sdk.resolve(),'--output',apk,'--id',package,'--version-code','42','--version-name','2 & next','--title','Juego <uno> & dos','--orientation','portrait','--min-api','30']
                if args.release:
                    if args.format == 'aab': arguments += ['--bundle-tools',args.bundle_tools.resolve()]
                    unsigned = apk.with_suffix('.unsigned.'+args.format)
                    unsigned_arguments = [unsigned if value == apk else value for value in arguments] + ['--release']
                    export(unsigned_arguments, args.work/(mode+'-unsigned-export.log'), cwd=project)
                    verify = subprocess.run((['jarsigner','-J-Duser.language=en','-verify','-strict',str(unsigned)] if args.format == 'aab' else [str(args.android_sdk.resolve()/'build-tools/34.0.0/apksigner'), 'verify', str(unsigned)]), capture_output=True, timeout=30)
                    if args.format == 'aab':
                        assert b'jar is unsigned' in verify.stdout, 'Unsigned AAB unexpectedly has a JAR signature'
                    else:
                        assert verify.returncode != 0, 'Unsigned release unexpectedly contains a signature'
                    unsigned.unlink()
                export(arguments+signing_arguments, args.work/(mode+'-export.log'),
                       cwd=project, env=signing_environment)
                published = apk
                prefix = 'base/' if args.format == 'aab' else ''
                with zipfile.ZipFile(apk) as archive:
                    assert archive.read(prefix+'assets/assets/value.bin')==b'\0\r\n\xff'
                    assert prefix+'assets/licenses/Lua-readme.html' in archive.namelist()
                    assert prefix+'assets/licenses/raylib-LICENSE' in archive.namelist()
                if args.format == 'aab':
                    from android_bundle_test import derive_apk
                    apk=project.parent/(package+'.apk')
                    derive_apk(published,apk,args.bundle_tools.resolve(),key,'test','MALVADO_TEST_STORE_PASSWORD',environment=signing_environment)
                metadata=command([args.android_sdk.resolve()/'build-tools/34.0.0/aapt','dump','badging',apk]).stdout
                assert b"versionCode='42'" in metadata and b"versionName='2 & next'" in metadata
                assert b"sdkVersion:'30'" in metadata
                (args.work/(mode+'-manifest.log')).write_bytes(metadata)
                if args.package_only:
                    assert (f"package: name='{package}'").encode() in metadata
                    assert (b'application-debuggable' in metadata) != args.release
                    results.append(dict(mode=mode, package=package, verified_package=True,
                                        format=args.format, release=args.release))
                    continue
                try:
                    command(adb+['install','--no-incremental',apk])
                    command(adb+['shell','am','wait-for-broadcast-idle'])
                    command(adb+['shell','am','start','-W','-n',package+'/'+GAME_ACTIVITY])
                    pid=command(adb+['shell','pidof',package]).stdout.decode().strip()
                    assert pid.isascii() and pid.isdigit()
                    deadline=time.monotonic()+30
                    red=mode=='error'
                    expected='game exited with status '+('1' if mode=='error' else '0')
                    while time.monotonic()<deadline:
                        logs=command(adb+['logcat','-d','--pid='+pid,'-s','MalvadoGame:I','*:S']).stdout
                        if expected.encode() in logs: break
                        if not red:
                            png=command(adb+['exec-out','screencap','-p']).stdout
                            pixel=center_pixel(png)
                            if pixel[0]>240 and pixel[1]<15 and pixel[2]<15:
                                (args.work/(mode+'.png')).write_bytes(png)
                                red=True
                                if args.text_input:
                                    command(adb+['shell','input','text','aZ0'])
                        time.sleep(.2)
                    else: raise RuntimeError('Exported game did not complete: '+logs.decode())
                    assert red, 'Exported game did not render red pixels'
                    assert b'game session cleanup passed' in logs
                    if args.text_input: assert b'exported Lua text input passed' in logs
                    if mode == 'error': assert b'expected exported process error' in logs
                    if args.release:
                        state=command(adb+['shell','dumpsys','package',package]).stdout
                        assert b'DEBUGGABLE' not in state
                        # Runtime status is nonzero if session cleanup fails.
                    else:
                        files=command(adb+['shell','run-as',package,'ls','files']).stdout
                        assert b'malvado-game-' not in files
                    (args.work/(mode+'-runtime.log')).write_bytes(logs)
                    results.append(dict(mode=mode,package=package,pid=pid,exit=1 if mode=='error' else 0,pixels=mode!='error',cleaned=True,release=args.release,text_input=args.text_input))
                finally:
                    command(adb+['uninstall',package])
                    remaining=command(adb+['shell','pm','list','packages',package]).stdout
                    assert ('package:'+package).encode() not in remaining
                    apk.unlink(missing_ok=True)
                    published.unlink(missing_ok=True)
        (args.work/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    if args.package_only:
        print('PASS: verified production Lua packages on the authoring host')
        return
    if args.text_input:
        print('PASS: exported explicit/declarative Lua physical-key text, pixels and cleanup')
        return
    print('PASS: exported explicit/declarative games, pixels, nested assets, process error and cleanup')


if __name__=='__main__': main()
