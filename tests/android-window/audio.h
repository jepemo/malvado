/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#ifndef MALVADO_TEST_ANDROID_AUDIO_H
#define MALVADO_TEST_ANDROID_AUDIO_H
#include <android/asset_manager.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
static MvSound *audio_sound;
static MvMusic *audio_music;
static MvSound *audio_finite;

static void
audio_begin(void)
{
    if (!ACCEPTANCE_AUDIO) return;
    struct android_app *app = GetAndroidApp();
    assert(app && chdir(app->activity->internalDataPath) == 0);
    const char *names[] = {"silence.wav", "silence.ogg", "empty.wav"};
    for (unsigned i = 0; i < 3; ++i) {
        AAsset *asset = AAssetManager_open(app->activity->assetManager, names[i], AASSET_MODE_BUFFER);
        assert(asset);
        off_t size = AAsset_getLength(asset);
        const void *bytes = AAsset_getBuffer(asset);
        assert(bytes && size > 0 && size < 1024 * 1024);
        FILE *file = fopen(names[i], "wb");
        assert(file && fwrite(bytes, 1, (size_t)size, file) == (size_t)size);
        assert(fclose(file) == 0);
        AAsset_close(asset);
    }
    assert(!mv_audio_update() && !mv_sound_load("silence.wav"));
    assert(mv_audio_open() && mv_audio_open());
    assert(!mv_sound_load("missing.wav") && !mv_sound_load("empty.wav"));
    assert(!mv_music_load("missing.wav", 0) && !mv_music_load("empty.wav", 0));
    assert(!mv_music_load("silence.wav", 2));
    MvSound *original = mv_sound_load("silence.wav");
    assert(original && !mv_sound_paused(original));
    assert(mv_sound_pause(original) && !mv_sound_paused(original));
    assert(!mv_sound_set_volume(original, -1) && !mv_sound_set_volume(original, NAN));
    audio_sound = mv_sound_clone(original);
    assert(audio_sound && mv_sound_play(audio_sound) && mv_sound_pause(audio_sound));
    mv_sound_destroy(original);
    if (!ACCEPTANCE_LIFECYCLE) {
        audio_finite = mv_sound_load("silence.wav");
        assert(audio_finite && mv_sound_play(audio_finite));
    }
    audio_music = mv_music_load("silence.ogg", 1);
    assert(audio_music && mv_music_play(audio_music) && mv_music_pause(audio_music));
}

static void
audio_frame(void)
{
    if (!ACCEPTANCE_AUDIO) return;
    assert(IsAudioDeviceReady() && mv_audio_update());
    assert(mv_sound_paused(audio_sound) && !mv_sound_playing(audio_sound));
    assert(mv_music_paused(audio_music) && !mv_music_playing(audio_music));
}

static void
audio_finish(void)
{
    if (!ACCEPTANCE_AUDIO) return;
    assert(mv_sound_resume(audio_sound) && mv_sound_playing(audio_sound));
    assert(mv_music_resume(audio_music) && mv_music_playing(audio_music));
    assert(mv_sound_stop(audio_sound) && !mv_sound_playing(audio_sound));
    assert(mv_music_stop(audio_music) && !mv_music_playing(audio_music));
    if (audio_finite) {
        assert(!mv_sound_playing(audio_finite));
        mv_sound_destroy(audio_finite);
    }
    mv_sound_destroy(audio_sound);
    mv_music_destroy(audio_music);
    mv_audio_close();
    mv_audio_close();
    assert(!IsAudioDeviceReady() && !mv_audio_update());
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s audio API passed", ACCEPTANCE_TOKEN);
}
#endif
