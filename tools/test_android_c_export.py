#!/usr/bin/env python3
"""Build/export ordinary installed-SDK C projects and execute their APKs."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from android_export import GAME_ACTIVITY
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
#ifdef EXPECT_AUDIO_FOCUS
#include <raylib.h>
#include <android/log.h>
#endif

typedef struct {
    int frames, destroyed, characters;
    MvImage *image;
#ifdef EXPECT_IME
    int editor_opened, hidden_frames;
#endif
#ifdef EXPECT_AUDIO_FOCUS
    MvSound *paused;
    int focus_stage;
#endif
} State;
static void destroy(void *data) {
    State *state = data;
    mv_image_destroy(state->image);
#ifdef EXPECT_AUDIO_FOCUS
    mv_sound_destroy(state->paused);
#endif
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
#ifdef EXPECT_AUDIO_FOCUS
    if (!mv_sound_paused(state->paused)) return MV_STEP_ERROR;
    float gain = GetMasterVolume();
    if (state->focus_stage == 0 && gain == 1.0f) {
        state->focus_stage++;
        __android_log_write(ANDROID_LOG_INFO, "MalvadoGame", "focus initial gain");
    } else if (state->focus_stage == 1 && gain == 0.0f) {
        state->focus_stage++;
        __android_log_write(ANDROID_LOG_INFO, "MalvadoGame", "focus transient mute");
    } else if (state->focus_stage == 2 && gain == 1.0f) {
        state->focus_stage++;
        __android_log_write(ANDROID_LOG_INFO, "MalvadoGame", "focus transient gain");
    } else if (state->focus_stage == 3 && gain == 0.0f) {
        state->focus_stage++;
        __android_log_write(ANDROID_LOG_INFO, "MalvadoGame", "focus permanent mute");
    } else if (state->focus_stage == 4 && gain == 1.0f) {
        uint32_t character;
        if (mv_text_input_next(&character) && character == 'r') {
            state->focus_stage++;
            __android_log_write(ANDROID_LOG_INFO, "MalvadoGame", "focus returned after Home");
        }
    }
    return ++state->frames == 1800 ? MV_STEP_ERROR : MV_STEP_YIELD;
#elif defined(EXPECT_IME)
    uint32_t codepoint;
    const uint32_t expected[] = {0xe9, 0x1f600};
    while (mv_text_input_next(&codepoint)) {
        if (!state->editor_opened) {
            if (codepoint != 'k' || !mv_text_input_set_keyboard(1)) return MV_STEP_ERROR;
            state->editor_opened = 1;
            continue;
        }
        if (state->characters >= 2 || codepoint != expected[state->characters++]) return MV_STEP_ERROR;
        if (state->characters == 2 && !mv_text_input_set_keyboard(0)) return MV_STEP_ERROR;
    }
    if (state->characters == 2 && ++state->hidden_frames == 100) {
        fputs("exported C IME passed\n", stderr);
        return MV_STEP_DONE;
    }
    return ++state->frames == 1200 ? MV_STEP_ERROR : MV_STEP_YIELD;
#elif defined(EXPECT_TEXT)
    uint32_t codepoint;
    while (mv_text_input_next(&codepoint)) {
        if (state->characters >= 3 || codepoint != (uint32_t)"aZ0"[state->characters++]) return MV_STEP_ERROR;
    }
    if (state->characters == 3) {
        fputs("exported C text input passed\n", stderr);
        return MV_STEP_DONE;
    }
    return ++state->frames == 900 ? MV_STEP_ERROR : MV_STEP_YIELD;
#else
    return ++state->frames == 360 ? MV_STEP_DONE : MV_STEP_YIELD;
#endif
}
int main(void) {
    MvContext *ctx = mv_context_create(60, 4);
    if (!ctx) return 5;
    const char *error = NULL;
    MvAssetSource *source = mv_asset_source_open_for_game(ctx, "assets/value.bin", &error);
    size_t length = 0;
    const char *bytes = mv_asset_source_data(source, &length);
    if (!source || length != 4 || memcmp(bytes, "\0\r\n\xff", 4)) return 2;
    mv_asset_source_destroy(source);
    if (!mv_window_open(320, 240, "C exported game")) return 3;
    State state = {0};
    state.image = mv_image_load_for_game(ctx, "red.png");
    if (!state.image || mv_image_width(state.image) != 8) return 4;
#ifdef EXPECT_AUDIO_FOCUS
    if (!mv_audio_open()) return 9;
    state.paused = mv_sound_load_for_game(ctx, "silence.wav");
    if (!state.paused || !mv_sound_play(state.paused) || !mv_sound_pause(state.paused)) return 10;
#endif
    MvProcessId id;
    if (!ctx || mv_process_spawn(ctx, 0, 0, step, destroy, &state, &id) != MV_OK) return 5;
    MvResult result = MV_OK;
    while (mv_process_count(ctx) && !mv_window_should_close()) {
        mv_window_begin((MvColor){0,0,0,255});
        result = mv_process_tick(ctx);
        mv_window_end();
        if (result != MV_OK) break;
    }
#ifdef EXPECT_AUDIO_FOCUS
    int good = state.focus_stage == 5 && mv_window_should_close();
#else
    int good = state.destroyed == 1 && mv_process_count(ctx) == 0;
#endif
    mv_context_destroy(ctx);
#ifdef EXPECT_AUDIO_FOCUS
    mv_audio_close();
#endif
    mv_window_close();
    if (!good || state.destroyed != 1) return 6;
#ifdef EXPECT_ERROR
    return result == MV_PROCESS_ERROR ? 1 : 7;
#else
#if defined(EXPECT_AUDIO_FOCUS)
    return result == MV_OK && !IsAudioDeviceReady() ? 0 : 8;
#elif defined(EXPECT_IME)
    return result == MV_OK && state.hidden_frames == 100 ? 0 : 8;
#elif defined(EXPECT_TEXT)
    return result == MV_OK && state.characters == 3 ? 0 : 8;
#else
    return result == MV_OK && state.frames == 360 ? 0 : 8;
#endif
#endif
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cli', 'runtime-sdk', 'android-sdk', 'work'):
        parser.add_argument('--'+name, required=True, type=Path)
    runner = parser.add_mutually_exclusive_group(required=True)
    runner.add_argument('--serial')
    runner.add_argument('--package-only', action='store_true',
                        help='Verify production packages on the authoring host without an emulator')
    parser.add_argument('--release', action='store_true')
    behavior = parser.add_mutually_exclusive_group()
    behavior.add_argument('--text-input', action='store_true')
    behavior.add_argument('--audio-focus', action='store_true')
    behavior.add_argument('--ime', action='store_true')
    parser.add_argument('--format', choices=('apk','aab'), default='apk')
    parser.add_argument('--bundle-tools', type=Path)
    parser.add_argument('--keystore', type=Path)
    parser.add_argument('--key-alias')
    parser.add_argument('--store-password-env')
    args = parser.parse_args()
    if args.package_only and (args.text_input or args.audio_focus or args.ime):
        parser.error('Runtime behavior verification requires an emulator serial')
    if args.format == 'aab' and (not args.release or not args.bundle_tools):
        parser.error('AAB verification requires release and bundle tools')
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
    if not args.package_only:
        checked(adb+['shell','input','keyevent','224'])
        checked(adb+['shell','wm','dismiss-keyguard'])
    results = []
    for mode in (('normal',) if args.text_input or args.audio_focus or args.ime else ('normal', 'process-error', 'compile-error', 'bad-entry')):
        with tempfile.TemporaryDirectory(prefix='C game with spaces ', dir=args.work.resolve()) as folder:
            project = Path(folder)/'project'
            checked([cli,'new','--c',project])
            cmake = project/'CMakeLists.txt'
            cmake.write_text(cmake.read_text().replace('COMPONENTS graphics)', 'COMPONENTS graphics assets)')+'\ntarget_link_libraries(game PRIVATE Malvado::assets)\n')
            source = ('#define EXPECT_TEXT\n' if args.text_input else '')+GAME
            if args.ime: source = '#define EXPECT_IME\n'+source
            if args.audio_focus:
                source = '#define EXPECT_AUDIO_FOCUS\n'+source
                fixtures = Path(__file__).resolve().parents[1]/'tests/fixtures'
                shutil.copyfile(fixtures/'silence.wav', project/'silence.wav')
            if mode == 'process-error': source = '#define EXPECT_ERROR\n'+source
            if mode == 'compile-error': source = '#error expected exported compile error\n'
            if mode == 'bad-entry': source = 'int main(int argc, char **argv) {return argc;}\n'
            (project/'main.c').write_text(source)
            (project/'assets').mkdir()
            (project/'assets/value.bin').write_bytes(b'\0\r\n\xff')
            shutil.copyfile(Path(__file__).resolve().parents[1]/'tests/fixtures/red.png',project/'red.png')
            package = 'dev.malvado.cexport.p'+uuid.uuid4().hex
            apk = Path(folder)/('game.'+args.format)
            arguments = [cli,'export','--target','android-arm64','--format',args.format,'--runtime-sdk',args.runtime_sdk.resolve(),'--android-sdk',args.android_sdk.resolve(),'--output',apk,'--id',package]
            # Native release verification uses an explicit disposable signing key;
            # the APK still disables debugging and links a Release SDK.
            if args.release:
                arguments += ['--release', '--keystore', args.keystore.resolve(), '--key-alias', args.key_alias, '--store-password-env', args.store_password_env]
            if args.format == 'aab': arguments += ['--bundle-tools', args.bundle_tools.resolve()]
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
            published = apk
            prefix = 'base/' if args.format == 'aab' else ''
            with zipfile.ZipFile(apk) as archive:
                assert archive.read(prefix+'assets/assets/value.bin') == b'\0\r\n\xff'
                assert prefix+'assets/main.c' not in archive.namelist()
                assert prefix+'assets/CMakeLists.txt' not in archive.namelist()
                assert prefix+'assets/licenses/raylib-LICENSE' in archive.namelist()
                notice = args.runtime_sdk/'share/licenses/malvado/raylib-NOTICES.txt'
                assert archive.read(prefix+'assets/licenses/raylib-NOTICES.txt') == notice.read_bytes()
            if args.format == 'aab':
                from android_bundle_test import derive_apk
                apk = Path(folder)/'game.apk'
                derive_apk(published,apk,args.bundle_tools.resolve(),args.keystore.resolve(),args.key_alias,args.store_password_env)
            if args.package_only:
                metadata = checked([args.android_sdk.resolve()/'build-tools/34.0.0/aapt',
                                    'dump', 'badging', apk]).stdout
                (args.work/(mode+'-manifest.log')).write_bytes(metadata)
                assert (f"package: name='{package}'").encode() in metadata
                assert (b'application-debuggable' in metadata) != args.release
                results.append(dict(mode=mode, package=package, verified_package=True,
                                    format=args.format, release=args.release))
                continue
            competitor = None
            try:
                if args.audio_focus:
                    from android_audio_focus_test import companion, exercise
                    focus_work = Path(folder)/'audio-focus'
                    focus_work.mkdir()
                    competitor, focus_apk = companion(checked, args.android_sdk.resolve(), focus_work)
                    checked(adb+['install','--no-incremental',focus_apk])
                checked(adb+['install','--no-incremental',apk])
                if args.release:
                    assert b'DEBUGGABLE' not in checked(adb+['shell','dumpsys','package',package]).stdout
                checked(adb+['shell','am','wait-for-broadcast-idle'])
                checked(adb+['shell','am','start','-W','-n',package+'/'+GAME_ACTIVITY])
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
                            if args.text_input: checked(adb+['shell','input','text','aZ0'])
                            if args.ime:
                                from test_android_ime import keyboard_sequence
                                keyboard_sequence(checked, adb, args.work, mode, pid)
                                deadline = time.monotonic()+15
                            if args.audio_focus:
                                exercise(checked, adb, package, pid, competitor, args.work, GAME_ACTIVITY)
                                deadline = time.monotonic()+15
                    time.sleep(.2)
                else: raise RuntimeError('Exported C game timed out: '+logs.decode())
                assert red and b'game session cleanup passed' in logs
                if args.text_input: assert b'exported C text input passed' in logs
                if args.ime: assert b'exported C IME passed' in logs
                if mode=='process-error': assert b'expected exported C process error' in logs
                (args.work/(mode+'-runtime.log')).write_bytes(logs)
                results.append(dict(mode=mode,package=package,pid=pid,cleaned=True,release=args.release,text_input=args.text_input, audio_focus=args.audio_focus, ime=args.ime))
            except (AssertionError, RuntimeError, subprocess.SubprocessError):
                diagnostics = command(adb + ['logcat', '-d', '-t', '300', '-s',
                                             'MalvadoGame:I', 'AndroidRuntime:E', 'raylib:D', '*:S'])
                (args.work/(mode+'-failure.log')).write_bytes(diagnostics.stdout+diagnostics.stderr)
                raise
            finally:
                for installed in (package, competitor):
                    if installed is not None:
                        command(adb+['uninstall',installed])
                        assert ('package:'+installed).encode() not in checked(adb+['shell','pm','list','packages',installed]).stdout
    (args.work/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    if args.package_only:
        print('PASS: verified production C packages on the authoring host')
        return
    if args.text_input:
        print('PASS: exported C physical-key text, pixels and cleanup')
        return
    if args.ime:
        print('PASS: exported C opens and closes the IME and receives committed Unicode')
        return
    if args.audio_focus:
        print('PASS: exported game audio focus, pause state, pixels and cleanup')
        return
    print('PASS: real C project export, scheduler, assets/pixels, process error, cleanup and rejected invalid builds')


if __name__=='__main__': main()
