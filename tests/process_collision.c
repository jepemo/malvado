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
#include <math.h>
#include "malvado/collision.h"

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    MvContext *other = mv_context_create(60, 5);
    assert(ctx && other);
    MvProcessId ids[4];
    MvProcessRef refs[4];
    for (int i = 0; i < 4; ++i) {
        assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &ids[i]) == MV_OK);
        assert(mv_process_ref_create(ctx, ids[i], &refs[i]) == MV_OK);
    }
    assert(mv_process_set_type(ctx, ids[1], 1) == MV_OK);
    assert(mv_process_set_type(ctx, ids[2], 1) == MV_OK);
    assert(mv_process_set_type(ctx, ids[3], 2) == MV_OK);
    MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = 2};
    MvCollisionTransform position = {0, 0, 0, 1};
    for (int i = 0; i < 4; ++i)
        assert(mv_process_collider_set(ctx, refs[i], circle, position) == MV_OK);
    MvProcessRef out[3];
    size_t count = 99;
    assert(mv_process_collider_find(ctx, refs[0], 1, out, 1, &count) == MV_OK);
    assert(count == 2 && out[0].id == ids[1]);
    MvProcessRef first;
    assert(mv_process_collider_first(ctx, refs[0], 1, &first) == MV_OK && first.id == ids[1]);
    first = refs[0];
    assert(mv_process_collider_first(other, refs[0], 1, &first) == MV_INVALID_ARGUMENT);
    assert(first.id == ids[0]);
    assert(mv_process_collider_first(ctx, refs[0], 1, &first) == MV_OK && first.id == ids[1]);
    MvProcessInfo info;
    assert(mv_process_info(ctx, first, &info) == MV_OK && info.id == ids[1] && info.type == 1);
    int hit = 99;
    assert(mv_process_collider_test(ctx, refs[0], refs[0], &hit) == MV_OK && !hit);
    assert(mv_process_collider_test(ctx, refs[0], refs[1], &hit) == MV_OK && hit);
    assert(mv_process_signal(ctx, ids[1], MV_SIGNAL_SLEEP, 0) == MV_OK);
    assert(mv_process_signal(ctx, ids[2], MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_collider_find(ctx, refs[0], 0, out, 3, &count) == MV_OK && count == 3);
    assert(out[0].id == ids[1] && out[1].id == ids[2] && out[2].id == ids[3]);
    assert(mv_process_collider_set(ctx, refs[1], circle, (MvCollisionTransform){4, 0, 0, 1}) == MV_OK);
    assert(mv_process_collider_test(ctx, refs[0], refs[1], &hit) == MV_OK && !hit);
    assert(mv_process_collider_set(ctx, refs[1], circle, (MvCollisionTransform){NAN, 0, 0, 1}) == MV_INVALID_ARGUMENT);
    assert(mv_process_collider_test(ctx, refs[0], refs[1], &hit) == MV_OK && !hit);
    assert(mv_process_collider_clear(ctx, refs[2]) == MV_OK);
    assert(mv_process_collider_clear(ctx, refs[2]) == MV_OK);
    assert(mv_process_collider_find(ctx, refs[0], 0, out, 3, &count) == MV_OK && count == 1 && out[0].id == ids[3]);
    assert(mv_process_collider_set(other, refs[0], circle, position) == MV_INVALID_ARGUMENT);
    int killed;
    assert(mv_process_kill(ctx, refs[3], &killed) == MV_OK && killed);
    assert(mv_process_collider_find(ctx, refs[0], 0, out, 3, &count) == MV_OK && count == 0);
    assert(mv_process_collider_first(ctx, refs[0], 0, &first) == MV_OK && first.id == 0);
    hit = 99;
    assert(mv_process_collider_test(ctx, refs[0], refs[3], &hit) == MV_NOT_FOUND && hit == 99);
    assert(mv_process_collider_clear(ctx, refs[3]) == MV_NOT_FOUND);
    assert(mv_process_collider_clear(ctx, refs[0]) == MV_OK);
    assert(mv_process_collider_find(ctx, refs[0], 0, out, 3, &count) == MV_OK && count == 0);
    assert(mv_process_collider_set(ctx, refs[0], circle, position) == MV_OK);
    unsigned steps;
    assert(mv_context_advance(ctx, 1.0 / 60.0, &steps) == MV_OK);
    MvProcessId replacement;
    MvProcessRef replacement_ref;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &replacement) == MV_OK);
    assert(mv_process_ref_create(ctx, replacement, &replacement_ref) == MV_OK);
    assert(mv_process_collider_set(ctx, replacement_ref, circle, position) == MV_OK);
    assert(mv_process_collider_find(ctx, refs[0], 0, out, 3, &count) == MV_OK && count == 1);
    assert(out[0].id == replacement && replacement != ids[3]);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
