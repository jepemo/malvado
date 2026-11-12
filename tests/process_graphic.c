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
#include "../src/core/process.h"
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

static void released(void *data) { ++*(int *)data; }

static void owned_visual_cleanup(void)
{
    MvContext *ctx = mv_context_create(60, 4);
    MvProcessId id;
    MvProcessRef ref;
    MvCollisionShape shape;
    assert(mv_graphic_sprite(3, 5, -2, 9, &shape));
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    int count = 0;
    assert(malvado_process_visual_set(ctx, ref, shape, &count, released) == MV_OK);
    assert(malvado_process_visual_set(ctx, ref, shape, &count, released) == MV_OK && count == 1);
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK && count == 2);
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK && count == 2);
    assert(malvado_process_visual_set(ctx, ref, shape, &count, released) == MV_OK);
    MvGraphic primitive = {.color = {255, 0, 0, 255}};
    assert(mv_graphic_circle(2, &primitive.shape));
    assert(mv_process_graphic_set(ctx, ref, primitive) == MV_OK && count == 3);
    assert(malvado_process_visual_set(ctx, ref, shape, &count, released) == MV_OK);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && count == 3);
    assert(mv_process_tick(ctx) == MV_OK && count == 4);
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    assert(malvado_process_visual_set(ctx, ref, shape, &count, released) == MV_OK);
    mv_context_destroy(ctx);
    assert(count == 5);
}

static void text_alignment(void)
{
    int out = 42;
    assert(!mv_text_aligned_x(0, -1, 0, &out) && out == 42);
    assert(!mv_text_aligned_x(0, 1, -1, &out) && out == 42);
    assert(!mv_text_aligned_x(0, 1, 3, &out) && out == 42);
    assert(!mv_text_aligned_x(0, 1, 0, NULL));
    assert(mv_text_aligned_x(INT_MAX, INT_MAX, 0, &out) && out == INT_MAX);
    assert(mv_text_aligned_x(10, 5, 1, &out) && out == 8);
    assert(mv_text_aligned_x(10, 5, 2, &out) && out == 5);
    assert(mv_text_aligned_x(INT_MIN, INT_MAX, 2, &out) && out == INT_MIN);
    assert(mv_text_aligned_x(INT_MIN, 0, 1, &out) && out == INT_MIN);
    assert(mv_text_aligned_x(INT_MAX, INT_MAX, 2, &out) && out == 0);
}

int main(void)
{
    text_alignment();
    owned_visual_cleanup();
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
