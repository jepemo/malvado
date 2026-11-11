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
#include "malvado/collision.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    MvCollisionWorld *world = mv_collision_world_create();
    assert(ctx && other && world);
    MvProcessId id;
    MvProcessRef ref;
    size_t hit;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    assert(mv_collision_world_add_rect(world, (MvRect){10, -10, 10, 20}, &hit));
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionSweep out = {1, 99, .25, 7, 8}, before = out;
    double x, y;
    assert(mv_process_world_move(ctx, ref, world, &shape, 100, 0, &out) == MV_OK);
    assert(out.hit && out.id == 1 && out.normal_x == -1 && out.normal_y == 0);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == 9 && y == 0);
    assert(mv_process_world_query(ctx, ref, world, &shape, &hit) == MV_OK && hit == 0);
    assert(mv_process_world_move(ctx, ref, world, &shape, 0, 2, &out) == MV_OK && !out.hit);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == 9 && y == 2);
    assert(mv_process_set_position(ctx, ref, 11, 0) == MV_OK);
    hit = 99;
    assert(mv_process_world_query(ctx, ref, world, &shape, &hit) == MV_OK && hit == 1);
    out = before;
    assert(mv_process_world_move(ctx, ref, world, &shape, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == 11 && y == 0);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    assert(mv_process_set_angle(ctx, ref, 90) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 2) == MV_OK);
    shape = (MvCollisionShape){.kind = MV_COLLISION_RECT, .width = 4, .height = 2};
    assert(mv_process_world_move(ctx, ref, world, &shape, 100, 0, &out) == MV_OK);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && fabs(x - 8) < 1e-12);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_world_move(ctx, ref, world, NULL, 100, 0, &out) == MV_OK && !out.hit && out.fraction == 1);
    assert(mv_process_world_query(ctx, ref, world, NULL, &hit) == MV_OK && hit == 0);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    out = before;
    hit = 99;
    assert(mv_process_world_move(other, ref, world, &shape, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query(other, ref, world, &shape, &hit) == MV_INVALID_ARGUMENT && hit == 99);
    assert(mv_process_world_move(NULL, ref, world, NULL, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_move(ctx, ref, NULL, NULL, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_move(ctx, ref, world, NULL, 1, 0, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query(ctx, ref, NULL, NULL, &hit) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query(ctx, ref, world, NULL, NULL) == MV_INVALID_ARGUMENT);
    double invalid[] = {NAN, INFINITY, -INFINITY, DBL_MAX};
    for (unsigned i = 0; i < 4; ++i) {
        assert(mv_process_world_move(ctx, ref, world, NULL, invalid[i], 0, &out) == MV_INVALID_ARGUMENT);
        assert(mv_process_world_move(ctx, ref, world, NULL, 0, invalid[i], &out) == MV_INVALID_ARGUMENT);
    }
    shape.width = -1;
    assert(mv_process_world_query(ctx, ref, world, &shape, &hit) == MV_INVALID_ARGUMENT && hit == 99);
    assert(mv_process_world_move(ctx, ref, world, &shape, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == 0 && y == 0);
    assert(mv_process_set_position(ctx, ref, DBL_MAX, 0) == MV_OK);
    assert(mv_process_world_move(ctx, ref, world, NULL, DBL_MAX / 16, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == DBL_MAX && y == 0);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    assert(mv_process_world_query(ctx, ref, world, NULL, &hit) == MV_NOT_FOUND && hit == 99);
    assert(mv_process_world_move(ctx, ref, world, NULL, 1, 0, &out) == MV_NOT_FOUND);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    mv_collision_world_destroy(world);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
