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
    int *stage = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color pixel = GetImageColor(image, image.width / 2, image.height / 2);
    if (*stage == 0 && pixel.r > 240 && pixel.g > 240 && pixel.b > 240)
        *stage = 1;
    else if (*stage == 1 && pixel.r < 10 && pixel.g < 10 && pixel.b < 10)
        *stage = 2;
    else if (*stage == 2 && pixel.r > 240 && pixel.g > 240 && pixel.b > 240)
        *stage = 3;
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int stage = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &stage);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && stage == 3);
    return 0;
}
