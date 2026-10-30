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
#include "malvado/platform.h"
#include "runner/runtime.h"

typedef struct { int red, green, blue_left, blue_right, yellow; } Seen;

static void
check_frame(void *user)
{
    Seen *seen = user;
    assert(mv_window_text_width("MMMM", 24) > 0);
    assert(mv_window_text_width(NULL, 24) == 0);
    assert(mv_window_text_width("Text", 0) == 0);
    Image image = LoadImageFromScreen();
    assert(image.data);
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            Color pixel = GetImageColor(image, x, y);
            if (x < 80 && pixel.r > 180 && pixel.g < 40 && pixel.b < 40)
                seen->red = 1;
            if (x >= 100 && pixel.g > 180 && pixel.r < 40 && pixel.b < 40)
                seen->green = 1;
            if (pixel.b > 180 && pixel.r < 40 && pixel.g < 40) {
                if (x < 120) seen->blue_left = 1;
                if (x > 120) seen->blue_right = 1;
            }
            if (pixel.r > 180 && pixel.g > 180 && pixel.b < 40) {
                assert(x <= 222);
                if (x >= 160) seen->yellow = 1;
            }
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
    assert(result == 0 && seen.red && seen.green && seen.blue_left && seen.blue_right && seen.yellow);
    return 0;
}
