#!/usr/bin/env python3
"""Export and execute ordinary Rust APK projects through the installed CLI."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
import uuid
from test_android_export import center_pixel
from android_export import GAME_ACTIVITY

GAME = r'''
use malvado::{Game, Error, SpawnOptions, image::Image, window::{Window, Color}};
use std::{rc::Rc, cell::Cell};
struct Guard(Rc<Cell<usize>>);
impl Drop for Guard {
    fn drop(&mut self) { self.0.set(self.0.get()+1); eprintln!("Rust exported resource released"); }
}
fn main() -> Result<(), Error> {
    assert_eq!(std::fs::read("assets/value.bin").unwrap(), &[0,13,10,255]);
    let window = Rc::new(Window::open(320,240,"Rust exported game")?);
    let image = Image::load(window.clone(), "red.png")?;
    let weak = Rc::downgrade(&window);
    let drops = Rc::new(Cell::new(0));
    let guard = Guard(drops.clone());
    if MODE == 2 { let _guard = guard; panic!("expected exported Rust panic"); }
    let mut game = Game::new()?;
    game.spawn(SpawnOptions::default(), move |p| async move {
        let _guard = guard;
        let _image = p.resource(image)?;
        if MODE == 1 { eprintln!("expected exported Rust process error"); return Err(Error::Process); }
        for _ in 0..(if MODE == 3 { 900 } else { 360 }) { p.frame().await; }
        Ok(())
    })?;
    let mut result = Ok(());
    let mut text = String::new();
    while game.process_count() != 0 {
        window.input_tick();
        if MODE == 3 {
            while let Some(character) = window.text_input_next() { text.push(character); }
            assert!("aZ0".starts_with(&text));
            if text == "aZ0" { eprintln!("exported Rust text input passed"); break; }
        }
        result = game.tick();
        if result.is_err() { break; }
        let _frame = window.frame(Color {r:255,g:0,b:0,a:255})?;
    }
    if MODE == 3 { assert_eq!(text,"aZ0"); }
    else { assert_eq!(game.process_count(),0); assert_eq!(drops.get(),1); }
    drop(game);
    assert_eq!(drops.get(),1);
    drop(window);
    assert!(weak.upgrade().is_none());
    result
}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('cli','runtime-sdk','android-sdk','work'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--serial',required=True)
    parser.add_argument('--release',action='store_true')
    parser.add_argument('--text-input',action='store_true')
    parser.add_argument('--format',choices=('apk','aab'),default='apk')
    parser.add_argument('--bundle-tools',type=Path)
    parser.add_argument('--keystore',type=Path)
    parser.add_argument('--key-alias')
    parser.add_argument('--store-password-env')
    args=parser.parse_args()
    if args.format=='aab' and (not args.release or not args.bundle_tools):
        parser.error('AAB verification requires release and bundle tools')
    if args.release and not all((args.keystore,args.key_alias,args.store_password_env)):
        parser.error('Release verification requires disposable signing credentials')
    args.work.mkdir(parents=True,exist_ok=True)
    (args.work/'results.json').unlink(missing_ok=True)
    cli=args.cli.resolve()
    sdk=args.android_sdk.resolve()
    adb=[str(sdk/'platform-tools/adb'),'-s',args.serial]
    environment=dict(os.environ,MALVADO_ANDROID_ADB=adb[0],MALVADO_ANDROID_SERIAL=args.serial)
    def command(arguments,**kwargs):
        return subprocess.run([str(value) for value in arguments],capture_output=True,timeout=360,env=environment,**kwargs)
    def checked(arguments,**kwargs):
        result=command(arguments,**kwargs)
        if result.returncode: raise RuntimeError(result.stdout.decode(errors='replace')+result.stderr.decode(errors='replace'))
        return result
    runner=args.work.resolve()/'probe runner'
    runner.write_text('#!/bin/sh\nexec '+shlex.join([sys.executable,str(Path(__file__).resolve().parent/'android_rust_runner.py')])+' "$@"\n')
    runner.chmod(0o755)
    checked(adb+['shell','input','keyevent','224'])
    checked(adb+['shell','wm','dismiss-keyguard'])
    records=[]
    for mode in (('normal',) if args.text_input else ('normal','process-error','panic','compile-error')):
        with tempfile.TemporaryDirectory(prefix='Rust game with spaces ',dir=args.work.resolve()) as folder:
            project=Path(folder)/'project'
            checked([cli,'new','--lang','rust',project])
            manifest=project/'Cargo.toml'
            manifest.write_text(manifest.read_text().replace('["graphics"]','["graphics", "assets"]'))
            code='const MODE: u8 = '+str(3 if args.text_input else {'normal':0,'process-error':1,'panic':2,'compile-error':0}[mode])+';\n'+GAME
            if mode=='compile-error': code='compile_error!("expected exported Rust compile error");\n'
            (project/'main.rs').write_text(code)
            (project/'assets').mkdir()
            (project/'assets/value.bin').write_bytes(b'\0\r\n\xff')
            shutil.copyfile(Path(__file__).resolve().parents[1]/'tests/fixtures/red.png',project/'red.png')
            package='dev.malvado.rustexport.p'+uuid.uuid4().hex
            apk=Path(folder)/('game.'+args.format)
            arguments=[cli,'export','--target','android-arm64','--format',args.format,'--runtime-sdk',args.runtime_sdk.resolve(),'--android-sdk',sdk,'--output',apk,'--id',package,'--probe-runner',runner]
            if args.release: arguments+=['--release','--keystore',args.keystore.resolve(),'--key-alias',args.key_alias,'--store-password-env',args.store_password_env]
            if args.format=='aab': arguments+=['--bundle-tools',args.bundle_tools.resolve()]
            exported=command(arguments,cwd=project)
            (args.work/(mode+'-build.log')).write_bytes(exported.stdout+exported.stderr)
            if mode=='compile-error':
                assert exported.returncode and not apk.exists()
                assert b'expected exported Rust compile error' in exported.stdout+exported.stderr
                assert not list(Path(folder).glob('.malvado-apk-*'))
                records.append(dict(mode=mode,rejected=True))
                continue
            if exported.returncode: raise RuntimeError(exported.stdout.decode()+exported.stderr.decode())
            if args.format=='aab':
                from android_bundle_test import derive_apk
                published=apk
                apk=Path(folder)/'game.apk'
                derive_apk(published,apk,args.bundle_tools.resolve(),args.keystore.resolve(),args.key_alias,args.store_password_env)
            try:
                checked(adb+['install','--no-incremental',apk])
                if args.release: assert b'DEBUGGABLE' not in checked(adb+['shell','dumpsys','package',package]).stdout
                checked(adb+['shell','am','wait-for-broadcast-idle'])
                checked(adb+['shell','am','start','-W','-n',package+'/'+GAME_ACTIVITY])
                pid=checked(adb+['shell','pidof',package]).stdout.decode().strip()
                assert pid.isascii() and pid.isdigit()
                deadline=time.monotonic()+30
                red=mode!='normal'
                marker=('game exited with status '+('0' if mode=='normal' else '1')).encode()
                while time.monotonic()<deadline:
                    logs=checked(adb+['logcat','-d','--pid='+pid,'-s','MalvadoGame:I','*:S']).stdout
                    if marker in logs: break
                    if not red:
                        png=checked(adb+['exec-out','screencap','-p']).stdout
                        pixel=center_pixel(png)
                        if pixel[0]>240 and pixel[1]<15 and pixel[2]<15:
                            (args.work/(mode+'.png')).write_bytes(png)
                            red=True
                            if args.text_input: checked(adb+['shell','input','text','aZ0'])
                    time.sleep(.2)
                else: raise RuntimeError('Rust exported game timed out: '+logs.decode())
                assert red and b'game session cleanup passed' in logs
                assert logs.count(b'Rust exported resource released')==1
                if args.text_input: assert b'exported Rust text input passed' in logs
                if mode=='process-error': assert b'expected exported Rust process error' in logs
                if mode=='panic': assert b'expected exported Rust panic' in logs
                (args.work/(mode+'-runtime.log')).write_bytes(logs)
                records.append(dict(mode=mode,package=package,pid=pid,cleaned=True,release=args.release,text_input=args.text_input))
            finally:
                checked(adb+['uninstall',package])
                assert ('package:'+package).encode() not in checked(adb+['shell','pm','list','packages',package]).stdout
    (args.work/'results.json').write_text(json.dumps(records,indent=2)+'\n')
    if args.text_input:
        print('PASS: exported Rust physical-key text, pixels and cleanup')
        return
    print('PASS: Rust APK scheduler/assets/pixels, process error, contained panic, cleanup and invalid build')


if __name__=='__main__': main()
