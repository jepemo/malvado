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
#include <assert.h>
#include <float.h>
#include <math.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int main(void)
{
    MvContext *ctx = mv_context_create(60, 4);
    MvContext *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    double x = 123, y = 456;
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == 0 && y == 0);
    assert(mv_process_set_position(ctx, ref, -DBL_MAX, DBL_MAX) == MV_OK);
    double invalid[] = {NAN, INFINITY, -INFINITY};
    for (unsigned i = 0; i < 3; ++i) {
        assert(mv_process_set_position(ctx, ref, invalid[i], 1) == MV_INVALID_ARGUMENT);
        assert(mv_process_set_position(ctx, ref, 1, invalid[i]) == MV_INVALID_ARGUMENT);
    }
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == -DBL_MAX && y == DBL_MAX);
    assert(mv_process_position(ctx, ref, &x, &x) == MV_INVALID_ARGUMENT && x == -DBL_MAX);
    assert(mv_process_position(ctx, ref, NULL, &y) == MV_INVALID_ARGUMENT);
    assert(mv_process_position(ctx, ref, &x, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_position(NULL, ref, &x, &y) == MV_INVALID_ARGUMENT);
    assert(mv_process_position(other, ref, &x, &y) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_position(other, ref, 1, 2) == MV_INVALID_ARGUMENT);
    assert(x == -DBL_MAX && y == DBL_MAX);
    assert(mv_process_set_position(ctx, ref, -1.5, 2.25) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_OK && x == -1.5 && y == 2.25);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    assert(mv_process_position(ctx, ref, &x, &y) == MV_NOT_FOUND && x == -1.5 && y == 2.25);
    assert(mv_process_set_position(ctx, ref, 0, 0) == MV_NOT_FOUND);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
