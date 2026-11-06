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
#include "malvado/platform.h"
#include "raylib.h"

int
main(void)
{
    assert(!mv_window_scroll_begin(0, 0));
    assert(!mv_window_scroll_end());
    assert(mv_window_open(64, 64, "Scroll test"));
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(!mv_window_scroll_begin(NAN, 0));
    assert(mv_window_clip_begin(10, 10, 20, 20));
    assert(mv_window_scroll_begin(10, 5));
    assert(!mv_window_scroll_begin(0, 0));
    assert(!mv_window_clip_end());
    assert(mv_window_draw_box(15, 10, 20, 20, 1, (MvColor){255, 0, 0, 255}));
    assert(mv_window_scroll_end());
    assert(!mv_window_scroll_end());
    assert(mv_window_clip_end());
    assert(mv_window_draw_box(40, 40, 5, 5, 1, (MvColor){0, 255, 0, 255}));
    mv_window_flush();
    Image image = LoadImageFromScreen();
    assert(image.data);
    int w = image.width, h = image.height;
    Color inside = GetImageColor(image, 12 * w / 64, 12 * h / 64);
    Color outside = GetImageColor(image, 7 * w / 64, 12 * h / 64);
    Color restored = GetImageColor(image, 42 * w / 64, 42 * h / 64);
    assert(inside.r > 240 && inside.g < 10);
    assert(outside.r < 10 && outside.g < 10);
    assert(restored.g > 240 && restored.r < 10);
    UnloadImage(image);
    assert(mv_window_scroll_begin(-2, -3));
    mv_window_end();
    assert(!mv_window_scroll_end());
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(mv_window_scroll_begin(0, 0));
    assert(mv_window_scroll_end());
    mv_window_end();
    mv_window_close();
    return 0;
}
