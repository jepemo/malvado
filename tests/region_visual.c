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
    MalvadoProcessInfo processes[4];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 4, &count, &tick));
    if (tick < 3 || *checked) return;
    assert(count == 4);
    Image image = LoadImageFromScreen();
    assert(image.data);
    int w = image.width, h = image.height;
    Color red = GetImageColor(image, 28 * w / 64, 15 * h / 64);
    Color old = GetImageColor(image, 22 * w / 64, 15 * h / 64);
    Color green = GetImageColor(image, 44 * w / 64, 12 * h / 64);
    Color blue = GetImageColor(image, 2 * w / 64, 43 * h / 64);
    Color off = GetImageColor(image, 7 * w / 64, 43 * h / 64);
    assert(red.r > 240 && red.g < 10);
    assert(old.r < 10 && old.g < 10 && old.b < 10);
    assert(green.g > 240 && green.r < 10);
    assert(blue.b > 240 && blue.r < 10);
    assert(off.b < 10 && off.r < 10);
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
