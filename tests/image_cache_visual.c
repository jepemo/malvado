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
#include <stdio.h>
#include <string.h>
#include "malvado/platform.h"
#include "raylib.h"

int main(int argc, char **argv)
{
    assert(argc == 2);
    assert(mv_window_open(64, 64, "Shared PNG textures"));
    MvImage *first = mv_image_load("tests/fixtures/red.png");
    MvImage *second = mv_image_load("./tests/fixtures/red.png");
    char absolute[4096];
    int size = snprintf(absolute, sizeof(absolute), "%s/tests/fixtures/red.png", GetWorkingDirectory());
    assert(size > 0 && (size_t)size < sizeof(absolute));
    MvImage *third = mv_image_load(absolute);
    assert(first && first == second && first == third);
    for (int i = 0; i < 50; ++i) {
        MvImage *extra = mv_image_load("tests/fixtures/red.png");
        assert(extra == first);
        mv_image_destroy(extra);
    }
    mv_image_destroy(first);
    mv_image_destroy(second);
    assert(mv_image_width(third) == 8 && mv_image_height(third) == 8);
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_image_draw(third, 10, 10);
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    Color red = GetImageColor(screen, 11 * screen.width / 64, 11 * screen.height / 64);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    UnloadImage(screen);
    mv_window_end();
    mv_image_destroy(third);
    /* A live reference is a snapshot. Releasing the last reference evicts it. */
    Image source = GenImageColor(2, 3, RED);
    assert(ExportImage(source, argv[1]));
    UnloadImage(source);
    MvImage *old = mv_image_load(argv[1]);
    assert(old && mv_image_width(old) == 2);
    source = GenImageColor(5, 4, BLUE);
    assert(ExportImage(source, argv[1]));
    UnloadImage(source);
    MvImage *retained = mv_image_load(argv[1]);
    assert(retained == old && mv_image_width(retained) == 2);
    mv_image_destroy(old);
    mv_image_destroy(retained);
    MvImage *fresh = mv_image_load(argv[1]);
    assert(fresh && mv_image_width(fresh) == 5 && mv_image_height(fresh) == 4);
    mv_image_destroy(fresh);
    assert(remove(argv[1]) == 0);
    assert(!mv_image_load("tests/fixtures/no-such-image.png"));
    mv_window_close();
    assert(mv_window_open(32, 32, "New image cache lifetime"));
    fresh = mv_image_load("tests/fixtures/red.png");
    assert(fresh && mv_image_width(fresh) == 8);
    mv_image_destroy(fresh);
    mv_window_close();
    return 0;
}
