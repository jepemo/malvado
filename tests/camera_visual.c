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

static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void check_frame(void *user)
{
    int *frames = user;
    if (++*frames != 3) return;
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color red = pixel(image, 20, 20), blue = pixel(image, 50, 70);
    Color green = pixel(image, 140, 20), yellow = pixel(image, 35, 75);
    Color clipped = pixel(image, 35, 65), old = pixel(image, 70, 50);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(blue.b > 240 && blue.r < 10);
    assert(green.g > 240 && green.r < 10);
    assert(yellow.r > 240 && yellow.g > 240 && yellow.b < 10);
    assert(clipped.r < 10 && clipped.g < 10 && clipped.b < 10);
    assert(old.r < 10 && old.g < 10 && old.b < 10);
    UnloadImage(image);
    MalvadoProcessInfo processes[4];
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 4, &total, &tick) && total == 4);
    for (size_t i = 0; i < total; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
}

int main(int argc, char **argv)
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
