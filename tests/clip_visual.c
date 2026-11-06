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
#include <limits.h>
#include "malvado/platform.h"
#include "raylib.h"

int
main(void)
{
    assert(!mv_window_clip_begin(0, 0, 1, 1));
    assert(!mv_window_clip_end());
    assert(mv_window_open(64, 64, "Clip test"));
    assert(!mv_window_clip_begin(0, 0, 1, 1));
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(!mv_window_clip_begin(-1, 0, 5, 5));
    assert(!mv_window_clip_begin(0, 0, 0, 5));
    assert(!mv_window_clip_begin(63, 0, INT_MAX, 5));
    assert(mv_window_clip_begin(10, 10, 20, 20));
    assert(!mv_window_clip_begin(0, 0, 64, 64));
    assert(mv_window_draw_box(0, 0, 64, 64, 1, (MvColor){255, 0, 0, 255}));
    assert(mv_window_clip_end());
    assert(!mv_window_clip_end());
    assert(mv_window_draw_box(40, 40, 5, 5, 1, (MvColor){0, 255, 0, 255}));
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int w = screen.width, h = screen.height;
    Color inside = GetImageColor(screen, 15 * w / 64, 15 * h / 64);
    Color outside = GetImageColor(screen, 5 * w / 64, 15 * h / 64);
    Color restored = GetImageColor(screen, 42 * w / 64, 42 * h / 64);
    assert(inside.r > 240 && inside.g < 10);
    assert(outside.r < 10 && outside.g < 10);
    assert(restored.g > 240 && restored.r < 10);
    UnloadImage(screen);
    assert(mv_window_clip_begin(0, 0, 1, 1));
    mv_window_end();
    assert(!mv_window_clip_end());
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(mv_window_clip_begin(0, 0, 64, 64));
    assert(mv_window_clip_end());
    mv_window_end();
    mv_window_close();
    assert(!mv_window_clip_begin(0, 0, 1, 1));
    return 0;
}
