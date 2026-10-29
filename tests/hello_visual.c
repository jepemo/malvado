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
#include "raylib.h"
#include "runner/runtime.h"

static void
check_frame(void *user)
{
    int *seen = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    for (int y = 80; y < 140 && y < image.height; ++y) {
        for (int x = 60; x < 400 && x < image.width; ++x) {
            Color color = GetImageColor(image, x, y);
            if (color.r > 180 && color.g > 180 && color.b > 180)
                *seen = 1;
        }
    }
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int seen = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &seen);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && seen);
    return 0;
}
