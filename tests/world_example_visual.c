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
    int *frames = user;
    if (++*frames != 50)
        return;
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color player = GetImageColor(screen, 284 * screen.width / width, 200 * screen.height / height);
    Color wall = GetImageColor(screen, 320 * screen.width / width, 200 * screen.height / height);
    assert(player.r == 80 && player.g == 190 && player.b == 255);
    assert(wall.r == 255 && wall.g == 165 && wall.b == 70);
    UnloadImage(screen);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int frames = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &frames);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && frames >= 50);
    return 0;
}
