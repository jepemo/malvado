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

typedef struct { int red, green; } Seen;

static void
check_frame(void *user)
{
    Seen *seen = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            Color pixel = GetImageColor(image, x, y);
            if (x < 80 && pixel.r > 180 && pixel.g < 40 && pixel.b < 40)
                seen->red = 1;
            if (x >= 100 && pixel.g > 180 && pixel.r < 40 && pixel.b < 40)
                seen->green = 1;
        }
    }
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    Seen seen = {0};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &seen);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && seen.red && seen.green);
    return 0;
}
