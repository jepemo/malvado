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

typedef struct {
    int seen;
} HelloCheck;

static void
check_frame(void *user)
{
    HelloCheck *check = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    int top = 300 * image.height / GetScreenHeight();
    int left = 400 * image.width / GetScreenWidth();
    int bottom = top + 60 * image.height / GetScreenHeight();
    int right = left + 340 * image.width / GetScreenWidth();
    for (int y = top; y < bottom && y < image.height; ++y) {
        for (int x = left; x < right && x < image.width; ++x) {
            Color color = GetImageColor(image, x, y);
            if (color.r < 80 && color.g < 80 && color.b < 80)
                check->seen = 1;
        }
    }
    UnloadImage(image);
    if (check->seen) {
        MalvadoProcessInfo process;
        size_t total;
        int64_t tick;
        assert(malvado_debug_snapshot(&process, 1, &total, &tick));
        assert(total == 1 && malvado_debug_signal(process.id, "kill"));
    }
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    HelloCheck check = {0};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &check);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && check.seen);
    return 0;
}
