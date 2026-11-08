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
    MalvadoProcessInfo processes[2];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 2, &count, &tick));
    if (tick < 3 || *checked) return;
    assert(count == 2);
    Image image = LoadImageFromScreen();
    assert(image.data);
    int w = image.width, h = image.height;
    Color transparent = GetImageColor(image, 25 * w / 128, 25 * h / 80);
    Color red = GetImageColor(image, 35 * w / 128, 25 * h / 80);
    Color green = GetImageColor(image, 55 * w / 128, 25 * h / 80);
    Color second_red = GetImageColor(image, 65 * w / 128, 25 * h / 80);
    assert(transparent.r == 25 && transparent.g == 30 && transparent.b == 45);
    assert(red.r >= 250 && red.g < 5 && red.b < 5);
    assert(green.g >= 250 && green.r < 5 && green.b < 5);
    assert(second_red.r >= 250 && second_red.g < 5 && second_red.b < 5);
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
