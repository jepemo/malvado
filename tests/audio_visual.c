/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine for Lua, inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "malvado/platform.h"
#include "runner/runtime.h"

static void
check_loop(MvMusic *music)
{
    if (!getenv("MALVADO_REAL_AUDIO"))
        return;
    double start, now;
    assert(mv_monotonic_time(&start));
    do {
        assert(mv_audio_update());
        assert(mv_monotonic_time(&now));
    } while (now - start < 0.75);
    assert(mv_music_playing(music));
}

static void debug_tick(void *user) { (void)user; }

typedef struct { MvMusic *music; int stage, frames; int64_t tick; } PumpCheck;
static void
check_paused_pump(void *user)
{
    PumpCheck *check = user;
    if (check->stage == 2) return;
    MalvadoProcessInfo info;
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(&info,0,&count,&tick));
    if (!check->stage) {
        check->music = mv_music_load("tests/fixtures/silence.wav",0);
        assert(check->music && mv_music_play(check->music));
        check->tick = tick;
        assert(malvado_debug_pause(1));
        check->stage = 1;
        return;
    }
    assert(tick == check->tick); /* The stream must finish without scheduler ticks. */
    assert(++check->frames < 90);
    if (!mv_music_playing(check->music)) {
        mv_music_destroy(check->music);
        check->music = NULL;
        check->stage = 2;
        assert(malvado_debug_pause(0));
    }
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    assert(!mv_audio_update());
    assert(!mv_sound_paused(NULL) && !mv_music_paused(NULL));
    assert(!mv_sound_load("tests/fixtures/silence.wav"));
    assert(mv_audio_open());
    assert(mv_audio_open());
    assert(!mv_sound_load("tests/fixtures/missing.wav"));
    assert(!mv_sound_load("tests/fixtures/silence.mp3"));
    MvSound *sound = mv_sound_load("tests/fixtures/silence.wav");
    assert(sound && !mv_sound_paused(sound));
    assert(mv_sound_pause(sound) && !mv_sound_paused(sound));
    assert(mv_sound_set_volume(sound, 0));
    assert(mv_sound_set_volume(sound, 1));
    assert(!mv_sound_set_volume(sound, -1));
    assert(!mv_sound_set_volume(sound, 2));
    assert(!mv_sound_set_volume(sound, NAN));
    assert(mv_sound_play(sound));
    assert(mv_sound_pause(sound));
    assert(mv_sound_paused(sound) && !mv_sound_playing(sound));
    assert(mv_sound_pause(sound) && mv_sound_paused(sound));
    assert(mv_sound_resume(sound) && !mv_sound_paused(sound));
    assert(mv_sound_stop(sound));
    assert(!mv_sound_playing(sound));
    MvSound *copy = mv_sound_clone(sound);
    assert(copy && mv_sound_play(sound) && mv_sound_play(copy));
    mv_sound_destroy(sound);
    assert(mv_sound_stop(copy) && mv_sound_play(copy));
    MvSound *second_copy = mv_sound_clone(copy);
    assert(second_copy && mv_sound_play(second_copy));
    mv_sound_destroy(copy);
    mv_sound_destroy(second_copy);
    assert(!mv_music_load("tests/fixtures/silence.wav", 2));
    assert(!mv_music_load("tests/fixtures/missing.wav", 0));
    assert(!mv_music_load("tests/fixtures/empty.wav", 0));
    MvMusic *music = mv_music_load("tests/fixtures/silence.wav", 1);
    assert(music);
    assert(mv_music_set_volume(music, 0.5f));
    assert(!mv_music_set_volume(music, NAN));
    assert(mv_music_play(music));
    assert(mv_music_update(music));
    check_loop(music);
    assert(mv_music_pause(music));
    assert(mv_music_paused(music) && !mv_music_playing(music));
    assert(mv_music_pause(music) && mv_music_paused(music));
    assert(mv_audio_update() && mv_music_paused(music));
    assert(mv_music_resume(music) && !mv_music_paused(music));
    assert(mv_music_stop(music));
    assert(!mv_music_playing(music) && !mv_music_paused(music));
    assert(mv_music_pause(music) && !mv_music_paused(music));
    assert(mv_audio_update() && !mv_music_playing(music));
    mv_music_destroy(music);
    assert(mv_audio_update());
    music = mv_music_load("tests/fixtures/silence.ogg", 1);
    assert(music && mv_music_play(music) && mv_music_update(music));
    check_loop(music);
    assert(mv_music_stop(music));
    mv_music_destroy(music);
    mv_audio_close();
    mv_audio_close();

    char *args[] = {"malvado-run", argv[1]};
    PumpCheck check = {0};
    malvado_debug_set_hook(debug_tick,NULL);
    malvado_debug_set_frame_hook(check_paused_pump,&check);
    assert(malvado_run(2, args) == 0);
    malvado_debug_set_frame_hook(NULL,NULL);
    malvado_debug_set_hook(NULL,NULL);
    /* The basic API script has no runner; the resource script exercises frames. */
    assert((check.stage == 0 || check.stage == 2) && !check.music);
    return 0;
}
