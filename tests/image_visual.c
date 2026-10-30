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
#include "malvado/platform.h"
#include "raylib.h"
#include "runner/runtime.h"

static void
check_pixels(void)
{
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    Color red = GetImageColor(screen, 12, 12);
    Color background = GetImageColor(screen, 30, 30);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(background.r < 10 && background.g < 10 && background.b < 10);
    UnloadImage(screen);
}

static void
check_frame(void *user)
{
    int *frames = user;
    check_pixels();
    ++*frames;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    assert(!mv_image_load("tests/fixtures/red.png"));
    assert(mv_window_open(64, 64, "C image test"));
    assert(!mv_image_load("tests/fixtures/missing.png"));
    assert(!mv_image_load("tests/fixtures/red.txt"));
    MvImage *image = mv_image_load("tests/fixtures/red.png");
    assert(image && mv_image_width(image) == 8 && mv_image_height(image) == 8);
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_image_draw(image, 10, 10);
    mv_window_flush();
    check_pixels();
    mv_window_end();
    mv_image_destroy(image);
    mv_image_destroy(NULL);
    mv_window_close();

    int frames = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &frames);
    int status = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(status == 0 && frames > 0);
    return 0;
}
