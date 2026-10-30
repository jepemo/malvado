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
#include <stdlib.h>
#include "malvado/platform.h"
#include "raylib.h"

static int calls, frames_after_stop, captured;

static int
test_space_down(void)
{
    return ++calls == 5;
}

static int
test_escape_down(void)
{
    return frames_after_stop >= 5;
}

static void
test_window_end(void)
{
    if (calls == 5) {
        const char *path = getenv("MALVADO_CAPTURE_PATH");
        if (path && !captured) {
            mv_window_flush();
            Image image = LoadImageFromScreen();
            captured = image.data && ExportImage(image, path);
            UnloadImage(image);
            assert(captured);
        }
        ++frames_after_stop;
    }
    mv_window_end();
}

#define mv_window_space_down test_space_down
#define mv_window_escape_down test_escape_down
#define mv_window_end test_window_end
#include "../examples/c/timing-game/main.c"
