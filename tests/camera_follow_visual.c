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
    ++*frames;
    if (*frames != 3 && *frames != 8) return;
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color red = pixel(image, 80, 60), blue = pixel(image, 65, 80);
    Color hud = pixel(image, 140, 20), empty = pixel(image, 115, 90);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(blue.b > 240 && blue.r < 10);
    assert(hud.g > 240 && hud.r < 10);
    assert(empty.r < 10 && empty.g < 10 && empty.b < 10);
    UnloadImage(image);
    if (*frames == 8) {
        MalvadoProcessInfo processes[3];
        size_t total;
        int64_t tick;
        assert(malvado_debug_snapshot(processes, 3, &total, &tick) && total == 3);
        for (size_t i = 0; i < total; ++i)
            assert(malvado_debug_signal(processes[i].id, "kill"));
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    int frames = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &frames);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && frames >= 8);
    return 0;
}
