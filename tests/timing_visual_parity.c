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
#include <stdlib.h>
#include "raylib.h"
#include "runner/runtime.h"

typedef struct {
    Image reference;
    int matched;
} Comparison;

static void
compare_frame(void *user)
{
    Comparison *comparison = user;
    if (comparison->matched)
        return;
    Image actual = LoadImageFromScreen();
    assert(actual.data);
    if (actual.width == comparison->reference.width && actual.height == comparison->reference.height) {
        int different = 0;
        for (int y = 0; y < actual.height; ++y) {
            for (int x = 0; x < actual.width; ++x) {
                Color a = GetImageColor(actual, x, y);
                Color b = GetImageColor(comparison->reference, x, y);
                if (abs((int)a.r - b.r) > 2 || abs((int)a.g - b.g) > 2 || abs((int)a.b - b.b) > 2)
                    ++different;
            }
        }
        comparison->matched = different == 0;
    }
    UnloadImage(actual);
}

int
main(int argc, char **argv)
{
    assert(argc == 3);
    Comparison comparison = {0};
    comparison.reference = LoadImage(argv[2]);
    assert(comparison.reference.data);
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(compare_frame, &comparison);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    UnloadImage(comparison.reference);
    assert(result == 0 && comparison.matched);
    return 0;
}
