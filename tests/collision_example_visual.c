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

typedef struct { int before, hit, after; } Seen;

static void
check_frame(void *user)
{
    Seen *seen = user;
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color wall = GetImageColor(screen, 450 * screen.width / width, 230 * screen.height / height);
    assert(wall.r > 240 && wall.g > 155 && wall.g < 175 && wall.b > 60 && wall.b < 80);
    int first_blue = -1;
    for (int x = 20; x < 620; ++x) {
        Color pixel = GetImageColor(screen, x * screen.width / width, 230 * screen.height / height);
        if (pixel.r >= 75 && pixel.r <= 85 && pixel.g >= 185 && pixel.g <= 195 && pixel.b > 245) {
            first_blue = x;
            break;
        }
    }
    int text = 0;
    for (int y = 55; y < 88 && !text; ++y) {
        for (int x = 20; x < 160; ++x) {
            Color pixel = GetImageColor(screen, x * screen.width / width, y * screen.height / height);
            if (pixel.r > 230 && pixel.g > 230 && pixel.b > 230) {
                text = 1;
                break;
            }
        }
    }
    if (first_blue >= 0 && !text && first_blue < 350 && !seen->hit) seen->before = 1;
    if (first_blue >= 0 && text && seen->before) seen->hit = 1;
    if (first_blue >= 0 && !text && first_blue < 350 && seen->hit) seen->after = 1;
    UnloadImage(screen);
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
    assert(result == 0 && seen.before && seen.hit && seen.after);
    return 0;
}
