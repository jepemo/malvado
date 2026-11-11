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
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvCollisionWorld *world = mv_collision_world_create();
    assert(world);
    size_t id;
    assert(mv_collision_world_add_rect(world, (MvRect){10, 0, 10, 10}, &id) && id == 1);
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionTransform position = {0, 5, 0, 1};
    MvContext *ctx = mv_context_create(60, 4);
    MvProcessId process;
    MvProcessRef ref;
    assert(ctx);
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &process) == MV_OK);
    assert(mv_process_ref_create(ctx, process, &ref) == MV_OK);
    assert(mv_process_set_position(ctx, ref, 0, 5) == MV_OK);
    assert(mv_process_collider_bind(ctx, ref, &shape) == MV_OK);
    MvCollisionSweep sweep;
    assert(mv_collision_world_sweep(world, shape, position, 100, 0, &sweep) == 1);
    assert(sweep.hit && sweep.id == 1 && sweep.normal_x == -1 && sweep.normal_y == 0);
    printf("sweep hit=1 fraction=%.6f normal=-1,0\n", sweep.fraction);
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &sweep) == MV_OK);
    assert(mv_process_position(ctx, ref, &position.x, &position.y) == MV_OK);
    assert(mv_process_world_query_selected(ctx, ref, world, &id) == MV_OK && id == 0);
    printf("move x=%.6f y=%.6f clear=1\n", position.x, position.y);
    assert(mv_process_world_move_selected(ctx, ref, world, 0, 1, &sweep) == MV_OK && !sweep.hit);
    assert(mv_process_position(ctx, ref, &position.x, &position.y) == MV_OK);
    printf("slide x=%.6f y=%.6f\n", position.x, position.y);
    assert(mv_process_set_position(ctx, ref, 11, 6) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, &sweep) == MV_INVALID_ARGUMENT);
    assert(mv_process_position(ctx, ref, &position.x, &position.y) == MV_OK);
    assert(position.x == 11 && position.y == 6);
    puts("failure unchanged=1");
    mv_context_destroy(ctx);
    mv_collision_world_destroy(world);
    return 0;
}
