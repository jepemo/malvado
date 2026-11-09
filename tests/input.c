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
    const uint8_t pixel[4] = {0, 0, 0, 255};
    assert(!mv_image_from_rgba(1, 1, pixel, sizeof(pixel)));
    assert(!mv_window_clip_begin(0, 0, 1, 1));
    assert(!mv_window_clip_end());
    assert(!mv_window_scroll_begin(0, 0));
    assert(!mv_window_scroll_end());
    assert(!mv_window_set_title(NULL) && !mv_window_set_title("No window"));
    int width = 1, height = 2;
    assert(!mv_window_size(NULL, &height) && height == 2);
    assert(!mv_window_size(&width, NULL) && width == 1);
    assert(!mv_window_size(&width, &height) && width == 0 && height == 0);
    assert(mv_window_text_width("Text", 20) == 0);
    assert(!mv_font_load("tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24));
    assert(!mv_font_text_width(NULL, "Text", 20));
    assert(!mv_font_draw(NULL, "Text", 0, 0, 20, (MvColor){255, 255, 255, 255}));
    mv_font_destroy(NULL);
    assert(!mv_window_draw_box(0, 0, 1, 1, 1, (MvColor){255, 255, 255, 255}));
    assert(!mv_window_draw_circle(0, 0, 1, 1, (MvColor){255, 255, 255, 255}));
    MvFade fade = {0};
    assert(!mv_fade_start(NULL, (MvColor){0}, 1));
    assert(mv_fade_start(&fade, (MvColor){20, 40, 80, 255}, 4));
    assert(mv_fade_active(&fade) && mv_fade_step(&fade));
    assert(fade.current.r == 5 && fade.current.a == 63);
    for (int i = 0; i < 3; ++i) assert(mv_fade_step(&fade));
    assert(!mv_fade_active(&fade) && !mv_fade_step(&fade));
    assert(fade.current.r == 20 && fade.current.a == 255);
    assert(!mv_fade_start(&fade, (MvColor){0}, 1000001) && fade.current.a == 255);
    assert(mv_fade_start(&fade, (MvColor){0}, 0) && fade.current.a == 0);
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
    for (int i = 0; i < 26; ++i) {
        char name[] = {(char)('a' + i), '\0'};
        MvKey key;
        assert(mv_key_from_name(name, &key) && key == (MvKey)(MV_KEY_A + i));
    }
    for (int i = 0; i < 10; ++i) {
        char name[] = {(char)('0' + i), '\0'};
        MvKey key;
        assert(mv_key_from_name(name, &key) && key == (MvKey)(MV_KEY_0 + i));
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
    static const struct { const char *name; MvPadButton value; } buttons[] = {
        {"a", MV_PAD_A}, {"b", MV_PAD_B}, {"x", MV_PAD_X}, {"y", MV_PAD_Y},
        {"up", MV_PAD_UP}, {"down", MV_PAD_DOWN}, {"left", MV_PAD_LEFT},
        {"right", MV_PAD_RIGHT}, {"lb", MV_PAD_LB}, {"rb", MV_PAD_RB},
        {"back", MV_PAD_BACK}, {"start", MV_PAD_START}
    };
    static const struct { const char *name; MvPadAxis value; } axes[] = {
        {"left_x", MV_PAD_LEFT_X}, {"left_y", MV_PAD_LEFT_Y},
        {"right_x", MV_PAD_RIGHT_X}, {"right_y", MV_PAD_RIGHT_Y},
        {"left_trigger", MV_PAD_LEFT_TRIGGER}, {"right_trigger", MV_PAD_RIGHT_TRIGGER}
    };
    MvPadButton pad_button = MV_PAD_A;
    MvPadAxis pad_axis = MV_PAD_LEFT_X;
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i)
        assert(mv_pad_button_from_name(buttons[i].name, &pad_button) && pad_button == buttons[i].value);
    for (size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i)
        assert(mv_pad_axis_from_name(axes[i].name, &pad_axis) && pad_axis == axes[i].value);
    assert(!mv_pad_button_from_name(NULL, &pad_button));
    assert(!mv_pad_button_from_name("a", NULL));
    assert(!mv_pad_axis_from_name(NULL, &pad_axis));
    assert(!mv_pad_axis_from_name("left_x", NULL));
    assert(mv_pad_button_from_name("start", &pad_button) && pad_button == MV_PAD_START);
    assert(!mv_pad_button_from_name("invalid", &pad_button) && pad_button == MV_PAD_START);
    assert(mv_pad_axis_from_name("right_y", &pad_axis) && pad_axis == MV_PAD_RIGHT_Y);
    assert(mv_pad_axis_from_name("left_trigger", &pad_axis) && pad_axis == MV_PAD_LEFT_TRIGGER);
    assert(!mv_pad_axis_from_name("invalid", &pad_axis) && pad_axis == MV_PAD_LEFT_TRIGGER);
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
    assert(!mv_sound_clone(NULL));
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
        uint32_t decoded = 0;
        assert(mv_utf8_decode(output, length, &decoded) == length && decoded == characters[i].codepoint);
    }
    /* Every Unicode scalar must survive an encode/decode round trip. */
    for (uint32_t scalar = 0; scalar <= 0x10ffff; ++scalar) {
        if (scalar >= 0xd800 && scalar <= 0xdfff)
            continue;
        uint32_t decoded = 0xffffffff;
        size_t length = mv_utf8_encode(scalar, output);
        assert(length >= 1 && length <= 4);
        assert(mv_utf8_decode(output, length, &decoded) == length);
        assert(decoded == scalar);
    }
    assert(!mv_utf8_encode(0xd800, output));
    assert(!mv_utf8_encode(0xdfff, output));
    assert(!mv_utf8_encode(0x110000, output));
    assert(!mv_utf8_encode(65, NULL));
    uint32_t unchanged_codepoint = 42;
    uint32_t zero = 42;
    assert(mv_utf8_decode("\0", 1, &zero) == 1 && zero == 0);
    assert(!mv_utf8_decode("\xc0\x80", 2, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode("\xed\xa0\x80", 3, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode("\xf4\x90\x80\x80", 4, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode("\xe2\x82", 2, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode("\xe2\x41\x80", 3, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode(NULL, 1, &unchanged_codepoint) && unchanged_codepoint == 42);
    assert(!mv_utf8_decode("A", 1, NULL));
    /* Reject truncated sequences and invalid continuation bytes at every position. */
    const char four_bytes[] = "\xf0\x90\x80\x80";
    for (size_t length = 0; length < 4; ++length) {
        assert(!mv_utf8_decode(four_bytes, length, &unchanged_codepoint));
        assert(unchanged_codepoint == 42);
    }
    for (size_t position = 1; position < 4; ++position) {
        char invalid[4];
        memcpy(invalid, four_bytes, sizeof(invalid));
        invalid[position] = 'A';
        assert(!mv_utf8_decode(invalid, sizeof(invalid), &unchanged_codepoint));
        assert(unchanged_codepoint == 42);
    }
    assert(!mv_utf8_decode("\xe0\x80\x80", 3, &unchanged_codepoint));
    assert(!mv_utf8_decode("\xf0\x80\x80\x80", 4, &unchanged_codepoint));
    assert(!mv_utf8_decode("\x80", 1, &unchanged_codepoint));
    assert(!mv_utf8_decode("\xf5\x80\x80\x80", 4, &unchanged_codepoint));
    assert(unchanged_codepoint == 42);
    assert(mv_utf8_decode("A\xff", 2, &zero) == 1 && zero == 'A');
    return 0;
}
