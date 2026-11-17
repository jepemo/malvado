#!/usr/bin/env python3
"""Build/export ordinary installed-SDK C projects and execute their APKs."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import zipfile
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
import uuid
from test_android_export import center_pixel

GAME = r'''
#include <malvado/platform.h>
#include <malvado/asset.h>
#include <stdio.h>
#include <string.h>

typedef struct { int frames, destroyed; MvImage *image; } State;
static void destroy(void *data) {
    State *state = data;
    mv_image_destroy(state->image);
    state->destroyed++;
}
static MvStepResult step(MvContext *ctx, MvProcessId id, void *data) {
    (void)ctx; (void)id;
    State *state = data;
#ifdef EXPECT_ERROR
    fputs("expected exported C process error\n", stderr);
    return MV_STEP_ERROR;
#endif
    if (!mv_window_draw_box(0, 0, 320, 240, 1, (MvColor){255,0,0,255})) return MV_STEP_ERROR;
    return ++state->frames == 360 ? MV_STEP_DONE : MV_STEP_YIELD;
}
int main(void) {
    const char *error = NULL;
    MvAssetSource *source = mv_asset_source_open("assets/value.bin", &error);
    size_t length = 0;
    const char *bytes = mv_asset_source_data(source, &length);
    if (!source || length != 4 || memcmp(bytes, "\0\r\n\xff", 4)) return 2;
    mv_asset_source_destroy(source);
    if (!mv_window_open(320, 240, "C exported game")) return 3;
    State state = {0};
    state.image = mv_image_load("red.png");
    if (!state.image || mv_image_width(state.image) != 8) return 4;
    MvContext *ctx = mv_context_create(60, 4);
    MvProcessId id;
    if (!ctx || mv_process_spawn(ctx, 0, 0, step, destroy, &state, &id) != MV_OK) return 5;
    MvResult result = MV_OK;
    while (mv_process_count(ctx) && !mv_window_should_close()) {
        mv_window_begin((MvColor){0,0,0,255});
        result = mv_process_tick(ctx);
        mv_window_end();
        if (result != MV_OK) break;
    }
    int good = state.destroyed == 1 && mv_process_count(ctx) == 0;
    mv_context_destroy(ctx);
    mv_window_close();
    if (!good || state.destroyed != 1) return 6;
#ifdef EXPECT_ERROR
    return result == MV_PROCESS_ERROR ? 1 : 7;
#else
    return result == MV_OK && state.frames == 360 ? 0 : 8;
#endif
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cli', 'runtime-sdk', 'android-sdk', 'work'):
        parser.add_argument('--'+name, required=True, type=Path)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--release', action='store_true')
    parser.add_argument('--keystore', type=Path)
    parser.add_argument('--key-alias')
    parser.add_argument('--store-password-env')
    args = parser.parse_args()
    if args.release and not all((args.keystore, args.key_alias, args.store_password_env)):
        parser.error('Release emulator checks require explicit test signing credentials')
    args.work.mkdir(parents=True, exist_ok=True)
    (args.work/'results.json').unlink(missing_ok=True)
    cli = args.cli.resolve()
    adb = [str(args.android_sdk.resolve()/'platform-tools/adb'), '-s', args.serial]
    def command(arguments, **kwargs):
        return subprocess.run([str(value) for value in arguments], capture_output=True, timeout=180, **kwargs)
    def checked(arguments, **kwargs):
        result = command(arguments, **kwargs)
        if result.returncode:
            raise RuntimeError(result.stdout.decode(errors='replace')+result.stderr.decode(errors='replace'))
        return result
    checked(adb+['shell','input','keyevent','224'])
    checked(adb+['shell','wm','dismiss-keyguard'])
    results = []
    for mode in ('normal', 'process-error', 'compile-error', 'bad-entry'):
        with tempfile.TemporaryDirectory(prefix='C game with spaces ', dir=args.work.resolve()) as folder:
            project = Path(folder)/'project'
            checked([cli,'new','--c',project])
            cmake = project/'CMakeLists.txt'
            cmake.write_text(cmake.read_text().replace('COMPONENTS graphics)', 'COMPONENTS graphics assets)')+'\ntarget_link_libraries(game PRIVATE Malvado::assets)\n')
            source = GAME
            if mode == 'process-error': source = '#define EXPECT_ERROR\n'+source
            if mode == 'compile-error': source = '#error expected exported compile error\n'
            if mode == 'bad-entry': source = 'int main(int argc, char **argv) {return argc;}\n'
            (project/'main.c').write_text(source)
            (project/'assets').mkdir()
            (project/'assets/value.bin').write_bytes(b'\0\r\n\xff')
            shutil.copyfile(Path(__file__).resolve().parents[1]/'tests/fixtures/red.png',project/'red.png')
            package = 'dev.malvado.cexport.p'+uuid.uuid4().hex
            apk = Path(folder)/'game.apk'
            arguments = [cli,'export','--target','android-arm64','--format','apk','--runtime-sdk',args.runtime_sdk.resolve(),'--android-sdk',args.android_sdk.resolve(),'--output',apk,'--id',package]
            # Native release verification uses an explicit disposable signing key;
            # the APK still disables debugging and links a Release SDK.
            if args.release:
                arguments += ['--release', '--keystore', args.keystore.resolve(), '--key-alias', args.key_alias, '--store-password-env', args.store_password_env]
            exported = command(arguments,cwd=project)
            (args.work/(mode+'-build.log')).write_bytes(exported.stdout+exported.stderr)
            if mode in ('compile-error','bad-entry'):
                assert exported.returncode != 0 and not apk.exists()
                assert not list(Path(folder).glob('.malvado-apk-*'))
                diagnostic = b'expected exported compile error' if mode == 'compile-error' else b'conflicting types'
                assert diagnostic in exported.stdout+exported.stderr
                results.append(dict(mode=mode,rejected=True))
                continue
            if exported.returncode: raise RuntimeError(exported.stdout.decode()+exported.stderr.decode())
            with zipfile.ZipFile(apk) as archive:
                assert archive.read('assets/assets/value.bin') == b'\0\r\n\xff'
                assert 'assets/main.c' not in archive.namelist()
                assert 'assets/CMakeLists.txt' not in archive.namelist()
                assert 'assets/licenses/raylib-LICENSE' in archive.namelist()
            try:
                checked(adb+['install','--no-incremental',apk])
                if args.release:
                    assert b'DEBUGGABLE' not in checked(adb+['shell','dumpsys','package',package]).stdout
                checked(adb+['shell','am','wait-for-broadcast-idle'])
                checked(adb+['shell','am','start','-W','-n',package+'/android.app.NativeActivity'])
                pid = checked(adb+['shell','pidof',package]).stdout.decode().strip()
                assert pid.isascii() and pid.isdigit()
                deadline=time.monotonic()+30
                red=mode=='process-error'
                marker=('game exited with status '+('1' if mode=='process-error' else '0')).encode()
                while time.monotonic()<deadline:
                    logs=checked(adb+['logcat','-d','--pid='+pid,'-s','MalvadoGame:I','*:S']).stdout
                    if marker in logs: break
                    if not red:
                        png=checked(adb+['exec-out','screencap','-p']).stdout
                        pixel=center_pixel(png)
                        if pixel[0]>240 and pixel[1]<15 and pixel[2]<15:
                            (args.work/(mode+'.png')).write_bytes(png)
                            red=True
                    time.sleep(.2)
                else: raise RuntimeError('Exported C game timed out: '+logs.decode())
                assert red and b'game session cleanup passed' in logs
                if mode=='process-error': assert b'expected exported C process error' in logs
                (args.work/(mode+'-runtime.log')).write_bytes(logs)
                results.append(dict(mode=mode,package=package,pid=pid,cleaned=True))
            finally:
                checked(adb+['uninstall',package])
                assert ('package:'+package).encode() not in checked(adb+['shell','pm','list','packages',package]).stdout
    (args.work/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    print('PASS: real C project export, scheduler, assets/pixels, process error, cleanup and rejected invalid builds')


if __name__=='__main__': main()
