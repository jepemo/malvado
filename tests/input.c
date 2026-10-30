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

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#else
#include <windows.h>
#endif
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "malvado/platform.h"

int
main(void)
{
    double before, after;
    assert(!mv_monotonic_time(NULL));
    assert(mv_monotonic_time(&before));
#ifdef _WIN32
    Sleep(30);
#else
    struct timespec delay = {0, 30000000};
    assert(nanosleep(&delay, NULL) == 0);
#endif
    assert(mv_monotonic_time(&after));
    assert(after - before >= 0.015);
    static const struct { const char *name; MvKey key; } cases[] = {
        {"escape", MV_KEY_ESCAPE}, {"space", MV_KEY_SPACE},
        {"left", MV_KEY_LEFT}, {"right", MV_KEY_RIGHT},
        {"up", MV_KEY_UP}, {"down", MV_KEY_DOWN},
        {"enter", MV_KEY_ENTER}, {"tab", MV_KEY_TAB}, {"backspace", MV_KEY_BACKSPACE},
        {"a", MV_KEY_A}, {"z", MV_KEY_Z}, {"0", MV_KEY_0}, {"9", MV_KEY_9}
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        MvKey key = MV_KEY_SPACE;
        assert(mv_key_from_name(cases[i].name, &key) && key == cases[i].key);
        assert(!mv_key_down(key));
    }
    MvKey unchanged = MV_KEY_A;
    assert(!mv_key_from_name("A", &unchanged) && unchanged == MV_KEY_A);
    assert(!mv_key_from_name("unknown", &unchanged) && unchanged == MV_KEY_A);
    assert(!mv_key_from_name(NULL, &unchanged));
    assert(!mv_key_from_name("left", NULL));
    assert(!mv_key_down((MvKey)-1));
    MvMouseButton button = MV_MOUSE_LEFT;
    assert(mv_mouse_button_from_name("left", &button) && button == MV_MOUSE_LEFT);
    assert(mv_mouse_button_from_name("middle", &button) && button == MV_MOUSE_MIDDLE);
    assert(mv_mouse_button_from_name("right", &button) && button == MV_MOUSE_RIGHT);
    assert(!mv_mouse_button_from_name("other", &button) && button == MV_MOUSE_RIGHT);
    assert(!mv_mouse_button_from_name(NULL, &button));
    assert(!mv_mouse_button_from_name("left", NULL));
    assert(!mv_mouse_down(MV_MOUSE_LEFT) && !mv_mouse_down((MvMouseButton)-1));
    MvPadButton pad_button = MV_PAD_A;
    MvPadAxis pad_axis = MV_PAD_LEFT_X;
    assert(mv_pad_button_from_name("start", &pad_button) && pad_button == MV_PAD_START);
    assert(!mv_pad_button_from_name("invalid", &pad_button) && pad_button == MV_PAD_START);
    assert(mv_pad_axis_from_name("right_y", &pad_axis) && pad_axis == MV_PAD_RIGHT_Y);
    assert(!mv_pad_axis_from_name("invalid", &pad_axis) && pad_axis == MV_PAD_RIGHT_Y);
    assert(!mv_pad_connected(-1) && !mv_pad_connected(4) && !mv_pad_connected(0));
    assert(!mv_pad_button_down(0, MV_PAD_A) && !mv_pad_button_down(-1, (MvPadButton)-1));
    assert(mv_pad_axis(0, MV_PAD_LEFT_X) == 0 && mv_pad_axis(-1, (MvPadAxis)-1) == 0);
    int x = 23, y = 42;
    assert(!mv_mouse_position(NULL, &y) && y == 42);
    assert(!mv_mouse_position(&x, NULL) && x == 23);
    assert(!mv_mouse_position(&x, &y) && x == 0 && y == 0);
    uint32_t codepoint = 123;
    assert(!mv_text_input_next(NULL));
    assert(!mv_text_input_next(&codepoint) && codepoint == 123);
    assert(!mv_sound_load("tests/fixtures/silence.wav"));
    assert(!mv_sound_play(NULL));
    assert(!mv_sound_stop(NULL));
    assert(!mv_sound_pause(NULL));
    assert(!mv_sound_resume(NULL));
    assert(!mv_sound_playing(NULL));
    assert(!mv_sound_set_volume(NULL, 0.5f));
    mv_sound_destroy(NULL);
    assert(!mv_music_load("tests/fixtures/silence.wav", 1));
    assert(!mv_music_play(NULL) && !mv_music_update(NULL) && !mv_music_stop(NULL));
    assert(!mv_music_pause(NULL) && !mv_music_resume(NULL) && !mv_music_playing(NULL));
    assert(!mv_music_set_volume(NULL, 0.5f));
    mv_music_destroy(NULL);
    static const struct { uint32_t codepoint; const char *utf8; size_t length; } characters[] = {
        {0x7f, "\x7f", 1}, {0x80, "\xc2\x80", 2}, {0x7ff, "\xdf\xbf", 2},
        {0x800, "\xe0\xa0\x80", 3}, {0xffff, "\xef\xbf\xbf", 3},
        {0x10000, "\xf0\x90\x80\x80", 4}, {0x10ffff, "\xf4\x8f\xbf\xbf", 4}
    };
    char output[4];
    for (size_t i = 0; i < sizeof(characters) / sizeof(characters[0]); ++i) {
        size_t length = mv_utf8_encode(characters[i].codepoint, output);
        assert(length == characters[i].length && memcmp(output, characters[i].utf8, length) == 0);
    }
    assert(!mv_utf8_encode(0xd800, output));
    assert(!mv_utf8_encode(0xdfff, output));
    assert(!mv_utf8_encode(0x110000, output));
    assert(!mv_utf8_encode(65, NULL));
    return 0;
}
