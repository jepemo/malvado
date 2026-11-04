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
    assert(GetScreenWidth() == 320 && GetScreenHeight() == 240);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color pixel = GetImageColor(image, 10 * image.width / 320, 10 * image.height / 240);
    if (pixel.r > 240 && pixel.g < 10 && pixel.b < 10)
        *seen = 1;
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
