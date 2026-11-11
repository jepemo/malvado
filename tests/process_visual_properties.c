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
    MvResult (*getters[])(MvContext *, MvProcessRef, double *) = {mv_process_angle, mv_process_scale};
    MvResult (*setters[])(MvContext *, MvProcessRef, double) = {mv_process_set_angle, mv_process_set_scale};
    for (unsigned i = 0; i < 2; ++i) {
        double out = -123;
        assert(getters[i](ctx, ref, &out) == MV_OK && out == (i ? 1 : 0));
        assert(getters[i](ctx, ref, NULL) == MV_INVALID_ARGUMENT);
        assert(getters[i](NULL, ref, &out) == MV_INVALID_ARGUMENT);
        assert(getters[i](other, ref, &out) == MV_INVALID_ARGUMENT);
        assert(setters[i](other, ref, 2) == MV_INVALID_ARGUMENT);
        assert(setters[i](NULL, ref, 2) == MV_INVALID_ARGUMENT);
        double values[] = {-DBL_MAX, DBL_MAX, -360.5, 0, .25};
        for (unsigned j = 0; j < 5; ++j) {
            assert(setters[i](ctx, ref, values[j]) == MV_OK);
            assert(mv_process_tick(ctx) == MV_OK);
            assert(getters[i](ctx, ref, &out) == MV_OK && out == values[j]);
        }
        double invalid[] = {NAN, INFINITY, -INFINITY};
        for (unsigned j = 0; j < 3; ++j) {
            assert(setters[i](ctx, ref, invalid[j]) == MV_INVALID_ARGUMENT);
            assert(getters[i](ctx, ref, &out) == MV_OK && out == .25);
        }
    }
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    for (unsigned i = 0; i < 2; ++i) {
        double out = 123;
        assert(getters[i](ctx, ref, &out) == MV_NOT_FOUND && out == 123);
        assert(setters[i](ctx, ref, 0) == MV_NOT_FOUND);
    }
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
