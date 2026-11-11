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
#include "malvado/graphic.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static void position(MvContext *ctx, MvProcessRef ref, double x, double y)
{
    double actual_x, actual_y;
    assert(mv_process_position(ctx, ref, &actual_x, &actual_y) == MV_OK);
    assert(actual_x == x && actual_y == y);
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    MvCollisionWorld *world = mv_collision_world_create(), *empty = mv_collision_world_create();
    assert(ctx && other && world && empty);
    size_t hit;
    assert(mv_collision_world_add_rect(world, (MvRect){10, -10, 1, 20}, &hit));
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    MvGraphic graphic = {.color = {255, 255, 255, 255}};
    assert(mv_graphic_rect(4, 2, 0, &graphic.shape));
    assert(mv_process_graphic_set(ctx, ref, graphic) == MV_OK);
    MvCollisionSweep out;
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &out) == MV_OK && out.hit);
    position(ctx, ref, 8, 0);
    assert(mv_process_world_query_selected(ctx, ref, world, &hit) == MV_OK && hit == 0);
    assert(mv_process_world_move_selected(ctx, ref, world, 0, 1, &out) == MV_OK && !out.hit);
    position(ctx, ref, 8, 1);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    assert(mv_process_set_angle(ctx, ref, 90) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 2) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &out) == MV_OK && out.hit);
    double x, y;
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && fabs(x - 8) < 1e-12);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = .5};
    assert(mv_process_collider_bind(ctx, ref, &circle) == MV_OK);
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &out) == MV_OK && out.hit);
    position(ctx, ref, 9, 0);
    assert(mv_process_collider_clear(ctx, ref) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 0) == MV_OK);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &out) == MV_OK && !out.hit && out.fraction == 1);
    position(ctx, ref, 109, 0);
    assert(mv_process_world_query_selected(ctx, ref, world, &hit) == MV_OK && !hit);
    assert(mv_process_collider_bind(ctx, ref, NULL) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, &out) == MV_OK && !out.hit);
    position(ctx, ref, 110, 0);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    circle.radius = 1;
    assert(mv_process_collider_set(ctx, ref, circle, (MvCollisionTransform){0, 0, 0, 1}) == MV_OK);
    assert(mv_process_set_position(ctx, ref, 100, 0) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, world, 100, 0, &out) == MV_OK && out.hit);
    position(ctx, ref, 109, 0);
    MvCollisionWorld *probe = mv_collision_world_create();
    assert(probe && mv_collision_world_add_rect(probe, (MvRect){8, -1, 2, 3}, &hit));
    assert(mv_process_world_query_selected(ctx, ref, probe, &hit) == MV_OK && hit == 1);
    assert(mv_process_set_position(ctx, ref, 500, 0) == MV_OK);
    assert(mv_process_world_query_selected(ctx, ref, probe, &hit) == MV_OK && hit == 1);
    assert(mv_process_world_move_selected(ctx, ref, world, 0, 1, &out) == MV_OK && !out.hit);
    position(ctx, ref, 500, 1);
    assert(mv_process_world_query_selected(ctx, ref, probe, &hit) == MV_OK && hit == 1);
    out = (MvCollisionSweep){1, 99, .25, 7, 8};
    MvCollisionSweep before;
    memcpy(&before, &out, sizeof(before));
    hit = 99;
    assert(mv_process_world_move_selected(other, ref, world, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query_selected(other, ref, world, &hit) == MV_INVALID_ARGUMENT && hit == 99);
    assert(mv_process_world_move_selected(NULL, ref, world, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query_selected(NULL, ref, world, &hit) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_move_selected(ctx, ref, NULL, 1, 0, &out) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query_selected(ctx, ref, NULL, &hit) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_world_query_selected(ctx, ref, world, NULL) == MV_INVALID_ARGUMENT);
    double invalid[] = {NAN, INFINITY, -INFINITY, DBL_MAX};
    for (unsigned i = 0; i < 4; ++i) {
        assert(mv_process_world_move_selected(ctx, ref, world, invalid[i], 0, &out) == MV_INVALID_ARGUMENT);
        assert(mv_process_world_move_selected(ctx, ref, world, 0, invalid[i], &out) == MV_INVALID_ARGUMENT);
    }
    position(ctx, ref, 500, 1);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_set_position(ctx, ref, DBL_MAX, 0) == MV_OK);
    assert(mv_process_collider_set(ctx, ref, circle, (MvCollisionTransform){9, 0, 0, 1}) == MV_OK);
    assert(mv_process_world_move_selected(ctx, ref, empty, DBL_MAX / 16, 0, &out) == MV_INVALID_ARGUMENT);
    position(ctx, ref, DBL_MAX, 0);
    assert(mv_process_world_query_selected(ctx, ref, probe, &hit) == MV_OK && hit == 1);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 1) == MV_OK);
    assert(mv_process_set_angle(ctx, ref, 0) == MV_OK);
    assert(mv_process_graphic_set(ctx, ref, graphic) == MV_OK);
    assert(mv_process_collider_bind(ctx, ref, NULL) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 0) == MV_OK);
    hit = 99;
    assert(mv_process_world_query_selected(ctx, ref, world, &hit) == MV_INVALID_ARGUMENT && hit == 99);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, &out) == MV_INVALID_ARGUMENT);
    position(ctx, ref, 0, 0);
    assert(mv_process_set_scale(ctx, ref, 1) == MV_OK);
    assert(mv_process_set_position(ctx, ref, 10, 0) == MV_OK);
    assert(mv_process_world_query_selected(ctx, ref, world, &hit) == MV_OK && hit == 1);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, &out) == MV_INVALID_ARGUMENT);
    position(ctx, ref, 10, 0);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    hit = 99;
    assert(mv_process_world_query_selected(ctx, ref, world, &hit) == MV_NOT_FOUND && hit == 99);
    assert(mv_process_world_move_selected(ctx, ref, world, 1, 0, &out) == MV_NOT_FOUND);
    mv_collision_world_destroy(probe);
    mv_collision_world_destroy(empty);
    mv_collision_world_destroy(world);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
