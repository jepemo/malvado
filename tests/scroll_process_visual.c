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
    int *checked = user;
    MalvadoProcessInfo processes[3];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 3, &count, &tick));
    if (tick < 3 || *checked) return;
    assert(count == 3);
    Image image = LoadImageFromScreen();
    assert(image.data);
    int w = image.width, h = image.height;
    Color red = GetImageColor(image, 12 * w / 64, 18 * h / 64);
    Color old = GetImageColor(image, 35 * w / 64, 22 * h / 64);
    Color hud = GetImageColor(image, 47 * w / 64, 12 * h / 64);
    Color blue = GetImageColor(image, 7 * w / 64, 43 * h / 64);
    Color outside = GetImageColor(image, 12 * w / 64, 43 * h / 64);
    assert(red.r > 240 && red.g < 10);
    assert(old.r < 10 && old.g < 10 && old.b < 10);
    assert(hud.g > 240 && hud.r < 10);
    assert(blue.b > 240 && blue.r < 10);
    assert(outside.b < 10 && outside.r < 10);
    UnloadImage(image);
    *checked = 1;
    for (size_t i = 0; i < count; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int checked = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &checked);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && checked);
    return 0;
}
