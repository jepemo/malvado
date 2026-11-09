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
#include <stdio.h>
#include "raylib.h"
#include "runner/runtime.h"
static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}
static void check_frame(void *user)
{
    int *checked = user;
    if (*checked) return;
    MalvadoProcessInfo processes[4];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 4, &count, &tick));
    if (tick < 4) return;
    assert(count == 4);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color center_red = pixel(image, 10, 10), center_green = pixel(image, 45, 10);
    Color default_red = pixel(image, 95, 25), default_outside = pixel(image, 80, 25);
    Color rotated_red = pixel(image, 40, 50), rotated_green = pixel(image, 40, 80);
    Color direct = pixel(image, 90, 80), top_left = pixel(image, 100, 80);
    assert(center_red.r > 240 && center_red.g < 10);
    assert(center_green.g > 240 && center_green.r < 10);
    assert(default_red.r > 240 && default_red.g < 10);
    assert(default_outside.r < 10 && default_outside.g < 10);
    assert(rotated_red.r > 240 && rotated_red.g < 10);
    assert(rotated_green.g > 240 && rotated_green.r < 10);
    assert(direct.r > 240 && top_left.r > 240);
    UnloadImage(image);
    *checked = 1;
    for (size_t i = 0; i < count; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    Image image = GenImageColor(5, 3, (Color){0, 255, 0, 255});
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 2; ++x) ImageDrawPixel(&image, x, y, (Color){255, 0, 0, 255});
    assert(ExportImage(image, argv[2]));
    UnloadImage(image);
    int checked = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &checked);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(remove(argv[2]) == 0);
    assert(result == 0 && checked);
    return 0;
}
