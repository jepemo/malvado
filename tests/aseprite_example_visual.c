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
    if (++*frames != 3)
        return;
    Image screen = LoadImageFromScreen();
    Image sheet = LoadImage("examples/aseprite/sheet.png");
    assert(screen.data && sheet.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color actual = GetImageColor(screen, 280 * screen.width / width, 190 * screen.height / height);
    Color expected = GetImageColor(sheet, 2, 2);
    Color outside = GetImageColor(screen, 240 * screen.width / width, 190 * screen.height / height);
    assert(actual.r == expected.r && actual.g == expected.g && actual.b == expected.b);
    assert(outside.r == 25 && outside.g == 30 && outside.b == 45);
    UnloadImage(sheet);
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
    assert(result == 0 && frames >= 3);
    return 0;
}
