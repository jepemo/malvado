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

typedef struct {
    int remaining;
    int destroyed;
} Counter;

static MvStepResult
count_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    Counter *counter = user;
    printf("tick: %d\n", counter->remaining);
    return --counter->remaining ? MV_STEP_YIELD : MV_STEP_DONE;
}

static void
count_destroy(void *user)
{
    Counter *counter = user;
    counter->destroyed++;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    Counter counter = {3, 0};
    MvProcessId id;
    if (mv_process_spawn(ctx, 0, 0, count_step, count_destroy, &counter, &id) != MV_OK) {
        mv_context_destroy(ctx);
        return 1;
    }
    while (mv_process_count(ctx)) {
        if (mv_process_tick(ctx) != MV_OK) {
            mv_context_destroy(ctx);
            return 1;
        }
    }
    int success = counter.remaining == 0 && counter.destroyed == 1;
    mv_context_destroy(ctx);
    return success ? 0 : 1;
}
