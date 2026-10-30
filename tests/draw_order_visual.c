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

typedef struct { int red, green, strict; } Seen;

static void
check_frame(void *user)
{
    Seen *seen = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color pixel = GetImageColor(image, 12, 12);
    if (seen->strict)
        assert((pixel.r > 240 && pixel.g < 10) || (pixel.g > 240 && pixel.r < 10));
    if (pixel.r > 240 && pixel.g < 10) {
        assert(!seen->green);
        seen->red = 1;
    }
    if (pixel.g > 240 && pixel.r < 10) {
        assert(seen->red);
        seen->green = 1;
    }
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2 || argc == 3);
    Seen seen = {0, 0, argc == 3};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &seen);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && seen.red && seen.green);
    return 0;
}
