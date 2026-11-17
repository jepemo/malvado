#!/usr/bin/env python3
"""Run real installed-CLI Lua APK exports on an explicit arm64 emulator."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
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
    parser.add_argument('--serial', required=True)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    (args.work / 'results.json').unlink(missing_ok=True)
    for mode in ('explicit', 'declarative', 'error'):
        (args.work / (mode + '.png')).unlink(missing_ok=True)
    cli = args.cli.resolve()
    adb = [str(args.android_sdk.resolve()/'platform-tools/adb'), '-s', args.serial]
    def command(values, **kwargs):
        return subprocess.run([str(value) for value in values], check=True, capture_output=True, timeout=120, **kwargs)
    command(adb+['shell','input','keyevent','224'])
    command(adb+['shell','wm','dismiss-keyguard'])
    results=[]
    for mode in ('explicit','declarative','error'):
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
 for i=1,360 do
  mv.set_color(255,0,0)
  mv.draw_box(0,0,320,240)
  mv.frame()
 end
end)
return function() task() end
''')
            entry="return {width=320,height=240,init=require('behavior')}" if mode=='declarative' else "require('malvado').start(require('behavior'),{width=320,height=240})"
            if mode=='error': entry='MODE_ERROR=true\n'+entry
            (project/'main.lua').write_text(entry)
            package='dev.malvado.exported.p'+uuid.uuid4().hex
            apk=project.parent/(package+'.apk')
            output=command([cli,'export','--target','android-arm64','--format','apk','--runtime-sdk',args.runtime_sdk.resolve(),'--android-sdk',args.android_sdk.resolve(),'--output',apk,'--id',package],cwd=project)
            (args.work/(mode+'-export.log')).write_bytes(output.stdout+output.stderr)
            with zipfile.ZipFile(apk) as archive:
                assert archive.read('assets/assets/value.bin')==b'\0\r\n\xff'
                assert 'assets/licenses/Lua-readme.html' in archive.namelist()
                assert 'assets/licenses/raylib-LICENSE' in archive.namelist()
            try:
                command(adb+['install','--no-incremental',apk])
                command(adb+['shell','am','wait-for-broadcast-idle'])
                command(adb+['shell','am','start','-W','-n',package+'/android.app.NativeActivity'])
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
                    time.sleep(.2)
                else: raise RuntimeError('Exported game did not complete: '+logs.decode())
                assert red, 'Exported game did not render red pixels'
                if mode == 'error': assert b'expected exported process error' in logs
                files=command(adb+['shell','run-as',package,'ls','files']).stdout
                assert b'malvado-game-' not in files
                (args.work/(mode+'-runtime.log')).write_bytes(logs)
                results.append(dict(mode=mode,package=package,pid=pid,exit=1 if mode=='error' else 0,pixels=mode!='error',cleaned=True))
            finally:
                command(adb+['uninstall',package])
                remaining=command(adb+['shell','pm','list','packages',package]).stdout
                assert ('package:'+package).encode() not in remaining
                apk.unlink(missing_ok=True)
    (args.work/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    print('PASS: exported explicit/declarative games, pixels, nested assets, process error and cleanup')


if __name__=='__main__': main()
