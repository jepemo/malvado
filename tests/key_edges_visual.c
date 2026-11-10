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
#include "runner/runtime.h"
#include "malvado/platform.h"
#include "input_internal.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

static void
capture(void *user)
{
    unsigned *tick = user;
    ++*tick;
    if (*tick == 1 || *tick == 4)
        mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    if (*tick == 3 || *tick == 4)
        mv_keyboard_sample(MV_KEY_SPACE, 0, 1);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    for (int run = 0; run < 2; ++run) {
        unsigned ticks = 0;
        malvado_debug_set_hook(capture, &ticks);
        char *args[] = {"malvado-run", argv[1]};
        assert(malvado_run(2, args) == 0);
        malvado_debug_set_hook(NULL, NULL);
        assert(ticks == 5);
        assert(!mv_key_pressed(MV_KEY_SPACE) && !mv_key_released(MV_KEY_SPACE));
    }
    return 0;
}
