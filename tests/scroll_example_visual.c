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
    if (tick < 2 || *checked) return;
    assert(count == 2);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color blue = GetImageColor(image, 70 * image.width / 640, 100 * image.height / 480);
    Color orange = GetImageColor(image, 210 * image.width / 640, 150 * image.height / 480);
    Color background = GetImageColor(image, 150 * image.width / 640, 100 * image.height / 480);
    assert(blue.b > 240 && blue.g > 180 && blue.r < 100);
    assert(orange.r > 240 && orange.g > 150 && orange.b < 90);
    assert(background.r < 40 && background.g < 45 && background.b < 60);
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
