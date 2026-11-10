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

int
main(int argc, char **argv)
{
    assert(argc == 2);
    /* Simulate another window host's pending input without opening a device. */
    mv_keyboard_open();
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    assert(malvado_run(3, args) == 0);
    mv_input_tick();
    assert(mv_key_pressed(MV_KEY_SPACE));
    mv_keyboard_reset();
    return 0;
}
