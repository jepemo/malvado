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
#include <stdio.h>
#include "malvado/malvado.h"

static MvStepResult
count_tick(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    unsigned *count = user;
    ++*count;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *game = mv_context_create(60, 5);
    if (!game)
        return 1;

    unsigned callbacks = 0;
    MvProcessId process;
    unsigned steps = 0;
    if (mv_process_spawn(game, 0, 0, count_tick, NULL, &callbacks, &process) != MV_OK ||
        mv_context_advance(game, 1.0 / 30.0, &steps) != MV_OK) {
        mv_context_destroy(game);
        return 1;
    }
    /* Advancing the clock does not run the scheduler. */
    int ok = steps == 2 && callbacks == 0 && mv_context_ticks(game) == 2;
    for (unsigned i = 0; i < steps; ++i) {
        if (mv_process_tick(game) != MV_OK)
            ok = 0;
    }
    printf("Hello, Malvado C: %u ticks\n", callbacks);
    ok = ok && callbacks == 2 && mv_context_dropped_seconds(game) == 0;
    mv_context_destroy(game);
    return ok ? 0 : 1;
}
