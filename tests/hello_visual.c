/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
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
    int stopped;
} HelloCheck;

static void
check_frame(void *user)
{
    HelloCheck *check = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    int top = 40 * image.height / GetScreenHeight();
    int left = 40 * image.width / GetScreenWidth();
    int bottom = top + 48 * image.height / GetScreenHeight();
    int right = left + 340 * image.width / GetScreenWidth();
    Color background = GetImageColor(image, 0, 0);
    assert(background.r < 80 && background.g < 80 && background.b < 80);
    for (int y = top; y < bottom && y < image.height; ++y) {
        for (int x = left; x < right && x < image.width; ++x) {
            Color color = GetImageColor(image, x, y);
            if (color.r > 200 && color.g > 200 && color.b > 200)
                check->seen = 1;
        }
    }
    UnloadImage(image);
    if (check->seen && !check->stopped) {
        MalvadoProcessInfo process;
        size_t total;
        int64_t tick;
        assert(malvado_debug_snapshot(&process, 1, &total, &tick));
        assert(total == 1 && malvado_debug_signal(process.id, "kill"));
        check->stopped = 1;
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
