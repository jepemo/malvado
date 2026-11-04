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
#include "malvado/collision.h"

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
    MvProcessRef root_ref, child_ref;
    assert(mv_process_ref_create(ctx, root, &root_ref) == MV_OK);
    assert(mv_process_ref_create(ctx, child, &child_ref) == MV_OK);
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionTransform transform = {0, 0, 0, 1};
    fail_allocation = 1;
    assert(mv_process_collider_set(ctx, root_ref, shape, transform) == MV_OK);
    assert(mv_process_collider_set(ctx, child_ref, shape, transform) == MV_OK);
    MvProcessRef matches[1];
    size_t match_count;
    assert(mv_process_collider_find(ctx, root_ref, 0, matches, 1, &match_count) == MV_OK);
    assert(match_count == 1 && matches[0].id == child);
    assert(mv_process_collider_clear(ctx, child_ref) == MV_OK);
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
