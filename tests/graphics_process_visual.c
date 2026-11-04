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
    Image image = LoadImageFromScreen();
    assert(image.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color red = GetImageColor(image, 15 * image.width / width, 30 * image.height / height);
    Color blue = GetImageColor(image, 90 * image.width / width, 40 * image.height / height);
    Color green = GetImageColor(image, 80 * image.width / width, 40 * image.height / height);
    Color background = GetImageColor(image, 25 * image.width / width, 30 * image.height / height);
    Color alpha = GetImageColor(image, 130 * image.width / width, 70 * image.height / height);
    Color outside = GetImageColor(image, 119 * image.width / width, 70 * image.height / height);
    assert(alpha.r >= 120 && alpha.r <= 135 && alpha.g >= 120 && alpha.g <= 135);
    assert(outside.r < 10 && outside.g < 10 && outside.b < 10);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(blue.b > 240 && blue.r < 10 && blue.g < 10);
    assert(green.g > 240 && green.r < 10 && green.b < 10);
    assert(background.r < 10 && background.g < 10 && background.b < 10);
    UnloadImage(image);
    MalvadoProcessInfo processes[4];
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 4, &total, &tick) && total == 4);
    for (size_t i = 0; i < total; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
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
