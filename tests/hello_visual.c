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
#include <string.h>
#include "raylib.h"
#include "runner/runtime.h"

typedef struct {
    int seen;
    int migrated;
} HelloCheck;

static void
check_frame(void *user)
{
    HelloCheck *check = user;
    Image image = LoadImageFromScreen();
    assert(image.data);
    int top = check->migrated ? 300 : 80;
    int left = check->migrated ? 400 : 60;
    for (int y = top; y < top + 60 && y < image.height; ++y) {
        for (int x = left; x < left + 340 && x < image.width; ++x) {
            Color color = GetImageColor(image, x, y);
            if (check->migrated ? (color.r < 80 && color.g < 80 && color.b < 80) :
                                  (color.r > 180 && color.g > 180 && color.b > 180))
                check->seen = 1;
        }
    }
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2 || (argc == 3 && strcmp(argv[2], "migrated") == 0));
    HelloCheck check = {0, argc == 3};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &check);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && check.seen);
    return 0;
}
