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

#include <assert.h>
#include <math.h>
#include "malvado/platform.h"
#include "runner/runtime.h"

int
main(int argc, char **argv)
{
    assert(argc == 2);
    assert(!mv_sound_load("tests/fixtures/silence.wav"));
    assert(mv_audio_open());
    assert(mv_audio_open());
    assert(!mv_sound_load("tests/fixtures/missing.wav"));
    assert(!mv_sound_load("tests/fixtures/silence.mp3"));
    MvSound *sound = mv_sound_load("tests/fixtures/silence.wav");
    assert(sound);
    assert(mv_sound_set_volume(sound, 0));
    assert(mv_sound_set_volume(sound, 1));
    assert(!mv_sound_set_volume(sound, -1));
    assert(!mv_sound_set_volume(sound, 2));
    assert(!mv_sound_set_volume(sound, NAN));
    assert(mv_sound_play(sound));
    assert(mv_sound_playing(sound));
    assert(mv_sound_pause(sound));
    assert(!mv_sound_playing(sound));
    assert(mv_sound_resume(sound));
    assert(mv_sound_playing(sound));
    assert(mv_sound_stop(sound));
    assert(!mv_sound_playing(sound));
    mv_sound_destroy(sound);
    mv_audio_close();
    mv_audio_close();

    char *args[] = {"malvado-run", argv[1]};
    assert(malvado_run(2, args) == 0);
    return 0;
}
