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
#include "malvado/malvado.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

enum { ROUNDS = 5000 };

typedef struct { int *destroyed; } ResourceData;

static void
release_resource(void *user)
{
    ResourceData *data = user;
    ++*data->destroyed;
}

static MvStepResult
wait_step(MvContext *ctx, MvProcessId id, void *user)
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
    int destroyed = 0;
    ResourceData data = {&destroyed};
    for (int round = 0; round < ROUNDS; ++round) {
        MvProcessId root, first, second, independent;
        assert(mv_process_spawn(ctx, 0, 0, wait_step, NULL, NULL, &root) == MV_OK);
        assert(mv_process_spawn(ctx, root, 0, wait_step, NULL, NULL, &first) == MV_OK);
        assert(mv_process_spawn(ctx, first, 0, wait_step, NULL, NULL, &second) == MV_OK);
        assert(mv_process_spawn_related(ctx, first, 0, 0, wait_step, NULL, NULL,
                                        &independent) == MV_OK);
        MvResource *owned[4];
        MvProcessId owners[] = {root, first, second, independent};
        for (int i = 0; i < 4; ++i) {
            MvProcessRef owner_ref;
            assert(mv_process_ref_create(ctx, owners[i], &owner_ref) == MV_OK);
            assert(mv_resource_create(ctx, &owner_ref, &data, release_resource, &owned[i]) == MV_OK);
        }
        assert(mv_resource_retain(owned[3]) == MV_OK);
        assert(mv_process_signal(ctx, first, MV_SIGNAL_FREEZE, 0) == MV_OK);
        assert(mv_process_signal(ctx, root, MV_SIGNAL_KILL, 0) == MV_OK);
        assert(mv_process_count(ctx) == 1);
        assert(mv_process_tick(ctx) == MV_OK);
        assert(destroyed == round * 4 + 3);
        size_t total;
        MvProcessInfo found;
        assert(mv_process_creation_descendants(ctx, root, 0, &found, 1, &total) == MV_OK);
        assert(total == 1 && found.id == independent && found.parent == 0);
        assert(mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_KILL) == MV_OK);
        assert(mv_process_tick(ctx) == MV_OK);
        assert(destroyed == round * 4 + 3);
        assert(mv_resource_release(owned[3]) == MV_OK);
        assert(destroyed == (round + 1) * 4);
        assert(mv_process_lineage_snapshot(ctx, NULL, 0, &total) == MV_OK && total == 0);
        assert(mv_process_count(ctx) == 0);
    }
    mv_context_destroy(ctx);
    assert(destroyed == ROUNDS * 4);
    return 0;
}
