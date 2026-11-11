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
#include <stdio.h>
#include <string.h>

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static void trace(MvContext *ctx, MvProcessRef source, const char *phase, MvProcessId expected)
{
    MvProcessRef out[2], first;
    size_t total;
    assert(mv_process_collider_find(ctx, source, 1, out, 2, &total) == MV_OK);
    assert(total == (expected ? 1u : 0u));
    assert(mv_process_collider_first(ctx, source, 1, &first) == MV_OK && first.id == expected);
    printf("%s hits=", phase);
    if (expected) { assert(out[0].id == expected); printf("%llu", (unsigned long long)out[0].id); }
    else printf("-");
    puts("");
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvProcessRef refs[3];
    MvGraphic graphic = {.color = {255, 255, 255, 255}};
    for (unsigned i = 0; i < 3; ++i) {
        MvProcessId id;
        assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
        assert(mv_process_ref_create(ctx, id, &refs[i]) == MV_OK);
        assert(mv_process_set_type(ctx, id, 1) == MV_OK);
        if (i) assert(mv_graphic_circle(1, &graphic.shape));
        else assert(mv_graphic_rect(4, 2, 0, &graphic.shape));
        assert(mv_process_graphic_set(ctx, refs[i], graphic) == MV_OK);
    }
    assert(mv_process_set_position(ctx, refs[1], 2, 0) == MV_OK);
    assert(mv_process_set_position(ctx, refs[2], 7.75, 0) == MV_OK);
    trace(ctx, refs[0], "initial", 2);
    assert(mv_process_set_position(ctx, refs[0], 10, 0) == MV_OK);
    trace(ctx, refs[0], "position", 3);
    assert(mv_process_set_angle(ctx, refs[0], 90) == MV_OK);
    trace(ctx, refs[0], "rotation", 0);
    assert(mv_process_set_scale(ctx, refs[0], 2) == MV_OK);
    trace(ctx, refs[0], "scale", 3);
    MvCollisionShape local = {.kind = MV_COLLISION_CIRCLE, .radius = .25};
    assert(mv_process_collider_bind(ctx, refs[0], &local) == MV_OK);
    local.radius = 100;
    trace(ctx, refs[0], "override", 0);
    assert(mv_process_collider_clear(ctx, refs[0]) == MV_OK);
    trace(ctx, refs[0], "disabled", 0);
    assert(mv_process_collider_bind(ctx, refs[0], NULL) == MV_OK);
    trace(ctx, refs[0], "restored", 3);
    assert(mv_process_graphic_clear(ctx, refs[0]) == MV_OK);
    trace(ctx, refs[0], "cleared", 0);
    assert(mv_graphic_rect(4, 2, 0, &graphic.shape));
    assert(mv_process_graphic_set(ctx, refs[0], graphic) == MV_OK);
    trace(ctx, refs[0], "reassigned", 3);
    assert(mv_process_set_angle(ctx, refs[0], 0) == MV_OK);
    assert(mv_process_set_scale(ctx, refs[0], 1) == MV_OK);
    assert(mv_process_set_position(ctx, refs[0], 0, 0) == MV_OK);
    MvCollisionWorld *world = mv_collision_world_create();
    size_t id;
    assert(world && mv_collision_world_add_rect(world, (MvRect){10, -10, 1, 20}, &id));
    MvCollisionSweep sweep;
    assert(mv_process_world_move(ctx, refs[0], world, &graphic.shape, 100, 0, &sweep) == MV_OK && sweep.hit);
    trace(ctx, refs[0], "world", 3);
    assert(mv_process_signal(ctx, refs[1].id, MV_SIGNAL_SLEEP, 0) == MV_OK);
    assert(mv_process_signal(ctx, refs[2].id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    trace(ctx, refs[0], "inactive", 3);
    assert(mv_process_set_scale(ctx, refs[2], 0) == MV_OK);
    trace(ctx, refs[0], "invalid-target", 0);
    int overlap = 99;
    assert(mv_process_collider_test(ctx, refs[0], refs[2], &overlap) == MV_INVALID_ARGUMENT && overlap == 99);
    assert(mv_process_set_scale(ctx, refs[2], 1) == MV_OK);
    assert(mv_process_set_scale(ctx, refs[0], 0) == MV_OK);
    MvProcessRef out = refs[0], first = refs[0];
    size_t total = 99;
    assert(mv_process_collider_find(ctx, refs[0], 1, &out, 1, &total) == MV_INVALID_ARGUMENT && total == 99 && out.id == refs[0].id);
    assert(mv_process_collider_first(ctx, refs[0], 1, &first) == MV_INVALID_ARGUMENT && first.id == refs[0].id);
    assert(mv_process_collider_clear(ctx, refs[0]) == MV_OK);
    assert(mv_process_collider_bind(ctx, refs[0], NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_collider_test(ctx, refs[0], refs[2], &overlap) == MV_OK && !overlap);
    assert(mv_process_set_scale(ctx, refs[0], 1) == MV_OK);
    assert(mv_process_collider_bind(ctx, refs[0], NULL) == MV_OK);
    assert(mv_process_collider_bind(other, refs[0], NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_collider_bind(NULL, refs[0], NULL) == MV_INVALID_ARGUMENT);
    local.radius = -1;
    assert(mv_process_collider_bind(ctx, refs[0], &local) == MV_INVALID_ARGUMENT);
    assert(mv_process_collider_test(ctx, refs[0], refs[2], &overlap) == MV_OK && overlap);
    /* A fixed collider ignores later visual properties until explicitly rebound. */
    assert(mv_process_collider_set(ctx, refs[0], graphic.shape, (MvCollisionTransform){8, 0, 0, 1}) == MV_OK);
    assert(mv_process_set_position(ctx, refs[0], -100, 0) == MV_OK);
    assert(mv_process_set_scale(ctx, refs[0], 0) == MV_OK);
    assert(mv_process_collider_bind(ctx, refs[0], NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_collider_test(ctx, refs[0], refs[2], &overlap) == MV_OK && overlap);
    int killed;
    assert(mv_process_kill(ctx, refs[0], &killed) == MV_OK && killed);
    assert(mv_process_collider_bind(ctx, refs[0], NULL) == MV_NOT_FOUND);
    mv_collision_world_destroy(world);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
