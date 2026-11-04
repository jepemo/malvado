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
#include "malvado/collision.h"
#include <stdio.h>
#include <time.h>

enum { PROCESSES = 1000, QUERIES = 1000 };

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}

static double
milliseconds(clock_t start, clock_t end)
{
    return (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    MvProcessRef refs[PROCESSES];
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 2};
    if (!ctx) return 1;
    for (int i = 0; i < PROCESSES; ++i) {
        MvProcessId id;
        if (mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) != MV_OK ||
            mv_process_ref_create(ctx, id, &refs[i]) != MV_OK ||
            mv_process_collider_set(ctx, refs[i], shape, (MvCollisionTransform){i * 16.0, 0, 0, 1}) != MV_OK)
            return 1;
    }
    clock_t start = clock();
    for (int i = 0; i < QUERIES; ++i) {
        MvProcessRef hit;
        if (mv_process_collider_first(ctx, refs[i % PROCESSES], 0, &hit) != MV_OK || hit.id)
            return 1;
    }
    clock_t misses = clock();
    for (int i = 0; i < PROCESSES; ++i)
        if (mv_process_collider_set(ctx, refs[i], shape, (MvCollisionTransform){0, 0, 0, 1}) != MV_OK)
            return 1;
    clock_t updates = clock();
    for (int i = 0; i < QUERIES; ++i) {
        MvProcessRef hit;
        if (mv_process_collider_first(ctx, refs[i % PROCESSES], 0, &hit) != MV_OK || !hit.id)
            return 1;
    }
    clock_t hits = clock();
    if (start == (clock_t)-1 || misses == (clock_t)-1 || updates == (clock_t)-1 || hits == (clock_t)-1)
        return 1;
    printf("processes=%d queries=%d misses_ms=%.3f updates_ms=%.3f hits_ms=%.3f\n",
           PROCESSES, QUERIES, milliseconds(start, misses), milliseconds(misses, updates),
           milliseconds(updates, hits));
    mv_context_destroy(ctx);
    return 0;
}
