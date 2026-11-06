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
    assert(screen.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color red = GetImageColor(screen, 12 * screen.width / width, 12 * screen.height / height);
    Color green = GetImageColor(screen, 22 * screen.width / width, 12 * screen.height / height);
    Color sprite = GetImageColor(screen, 50 * screen.width / width, 40 * screen.height / height);
    Color outside = GetImageColor(screen, 40 * screen.width / width, 40 * screen.height / height);
    Color cropped = GetImageColor(screen, 7 * screen.width / width, 49 * screen.height / height);
    Color rotated_crop = GetImageColor(screen, 50 * screen.width / width, 60 * screen.height / height);
    Color manual_crop = GetImageColor(screen, 23 * screen.width / width, 49 * screen.height / height);
    Color cropped_outside = GetImageColor(screen, 5 * screen.width / width, 46 * screen.height / height);
    Color legacy = GetImageColor(screen, 31 * screen.width / width, 20 * screen.height / height);
    Color legacy_outside = GetImageColor(screen, 40 * screen.width / width, 20 * screen.height / height);
    assert(rotated_crop.g > 240 && rotated_crop.r < 10 && rotated_crop.b < 10);
    assert(manual_crop.g > 240 && manual_crop.r < 10 && manual_crop.b < 10);
    assert(cropped.g > 240 && cropped.r < 10 && cropped.b < 10);
    assert(cropped_outside.r < 10 && cropped_outside.g < 10 && cropped_outside.b < 10);
    assert(legacy.r > 240 && legacy.g < 10 && legacy.b < 10);
    assert(legacy_outside.r < 10 && legacy_outside.g < 10 && legacy_outside.b < 10);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(green.g > 240 && green.r < 10 && green.b < 10);
    assert(sprite.g > 240 && sprite.r < 10 && sprite.b < 10);
    assert(outside.r < 10 && outside.g < 10 && outside.b < 10);
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
