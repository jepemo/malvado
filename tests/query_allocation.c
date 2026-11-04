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
#include "malvado/malvado.h"

static int fail_allocation;

void *test_query_malloc(size_t size)
{
    if (fail_allocation) return NULL;
    return malloc(size);
}

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    MvProcessId root, child, independent;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &root) == MV_OK);
    assert(mv_process_spawn(ctx, root, 0, idle, NULL, NULL, &child) == MV_OK);
    assert(mv_process_spawn_related(ctx, root, 0, 0, idle, NULL, NULL, &independent) == MV_OK);
    fail_allocation = 1;
    assert(mv_process_signal(ctx, root, MV_SIGNAL_FREEZE, 1) == MV_OUT_OF_MEMORY);
    assert(mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_KILL) == MV_OUT_OF_MEMORY);
    fail_allocation = 0;
    MvProcessInfo items[3];
    size_t total;
    assert(mv_process_snapshot(ctx, items, 3, &total) == MV_OK && total == 3);
    for (size_t i = 0; i < total; i++)
        assert(items[i].status == MV_PROCESS_RUNNING);
    assert(mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_FREEZE) == MV_OK);
    assert(mv_process_snapshot(ctx, items, 3, &total) == MV_OK && total == 3);
    for (size_t i = 0; i < total; i++)
        assert(items[i].status == MV_PROCESS_FROZEN);
    mv_context_destroy(ctx);
    return 0;
}
