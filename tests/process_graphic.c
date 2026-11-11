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
    assert(ctx && other);
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    MvGraphic value = {.color = {1, 2, 3, 4}};
    assert(mv_graphic_rect(4, 2, 1, &value.shape));
    MvGraphic out = value, before = value;
    int present = 99;
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_OK && !present);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_graphic_set(ctx, ref, value) == MV_OK);
    value.color.r = 100;
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_OK && present);
    assert(out.color.r == 1 && out.color.a == 4 && out.shape.offset_x == 2);
    out.color.g = 100;
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_OK && out.color.g == 2);
    MvGraphic invalid[] = {value, value, value, value};
    invalid[0].shape.width = NAN;
    invalid[1].shape.offset_x = 1;
    invalid[2].shape.kind = (MvCollisionKind)99;
    invalid[3].shape.height = -1;
    for (unsigned i = 0; i < 4; ++i)
        assert(mv_process_graphic_set(ctx, ref, invalid[i]) == MV_INVALID_ARGUMENT);
    present = 99;
    before = out;
    assert(mv_process_graphic_get(other, ref, &out, &present) == MV_INVALID_ARGUMENT && present == 99);
    assert(mv_process_graphic_set(other, ref, value) == MV_INVALID_ARGUMENT);
    assert(mv_process_graphic_clear(other, ref) == MV_INVALID_ARGUMENT);
    assert(mv_process_graphic_get(ctx, ref, NULL, &present) == MV_INVALID_ARGUMENT);
    assert(mv_process_graphic_get(ctx, ref, &out, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_graphic_get(NULL, ref, &out, &present) == MV_INVALID_ARGUMENT);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_graphic_circle(3, &value.shape));
    value.shape.offset_y = 1;
    assert(mv_process_graphic_set(ctx, ref, value) == MV_INVALID_ARGUMENT);
    value.shape.offset_y = 0;
    assert(mv_process_graphic_set(ctx, ref, value) == MV_OK);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_OK && present && out.shape.radius == 3);
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_OK && !present);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    before = out;
    present = 99;
    assert(mv_process_graphic_get(ctx, ref, &out, &present) == MV_NOT_FOUND && present == 99);
    assert(memcmp(&out, &before, sizeof(out)) == 0);
    assert(mv_process_graphic_set(ctx, ref, value) == MV_NOT_FOUND);
    assert(mv_process_graphic_clear(ctx, ref) == MV_NOT_FOUND);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
