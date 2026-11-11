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
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}
static void order(MvContext *ctx, const char *expected)
{
    MvProcessDrawInfo items[3];
    size_t total;
    assert(mv_process_draw_order(ctx, NULL, 0, &total) == MV_OK);
    assert(mv_process_draw_order(ctx, items, 3, &total) == MV_OK);
    char text[32] = {0};
    size_t used = 0;
    for (size_t i = 0; i < total; ++i)
        used += (size_t)snprintf(text + used, sizeof(text) - used, "%s%llu", i ? "," : "", (unsigned long long)items[i].id);
    assert(strcmp(text, expected) == 0);
    puts(text);
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvProcessRef refs[3];
    for (unsigned i = 0; i < 3; ++i) {
        MvProcessId id;
        assert(mv_process_spawn(ctx, 0, 100 - i, step, NULL, NULL, &id) == MV_OK);
        assert(mv_process_ref_create(ctx, id, &refs[i]) == MV_OK);
        double z = -1;
        assert(mv_process_z(ctx, refs[i], &z) == MV_OK && z == 0);
    }
    assert(mv_process_set_z(ctx, refs[0], 10) == MV_OK);
    order(ctx, "2,3,1");
    MvProcessDrawInfo out[2] = {{99, 123}, {98, 456}}, before[2];
    memcpy(before, out, sizeof(out));
    size_t total = 99;
    assert(mv_process_draw_order(ctx, out, 2, &total) == MV_BUFFER_TOO_SMALL && total == 3);
    assert(memcmp(before, out, sizeof(out)) == 0);
    total = 99;
    assert(mv_process_draw_order(ctx, NULL, 1, &total) == MV_INVALID_ARGUMENT && total == 99);
    assert(mv_process_draw_order(NULL, NULL, 0, &total) == MV_INVALID_ARGUMENT && total == 99);
    assert(mv_process_draw_order(ctx, NULL, 0, NULL) == MV_INVALID_ARGUMENT);
    double z = 123;
    assert(mv_process_z(other, refs[0], &z) == MV_INVALID_ARGUMENT && z == 123);
    assert(mv_process_set_z(other, refs[0], 2) == MV_INVALID_ARGUMENT);
    assert(mv_process_z(ctx, refs[0], NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_z(ctx, refs[0], NAN) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_z(ctx, refs[0], INFINITY) == MV_INVALID_ARGUMENT);
    assert(mv_process_z(ctx, refs[0], &z) == MV_OK && z == 10);
    assert(mv_process_set_z(ctx, refs[0], DBL_MAX) == MV_OK);
    assert(mv_process_set_z(ctx, refs[0], -1) == MV_OK);
    order(ctx, "1,2,3");
    assert(mv_process_signal(ctx, refs[1].id, MV_SIGNAL_SLEEP, 0) == MV_OK);
    assert(mv_process_signal(ctx, refs[2].id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    order(ctx, "1,3");
    assert(mv_process_signal(ctx, refs[1].id, MV_SIGNAL_WAKEUP, 0) == MV_OK);
    order(ctx, "1,2,3");
    int killed;
    assert(mv_process_kill(ctx, refs[0], &killed) == MV_OK && killed);
    order(ctx, "2,3");
    assert(mv_process_z(ctx, refs[0], &z) == MV_NOT_FOUND);
    assert(mv_process_set_z(ctx, refs[0], 0) == MV_NOT_FOUND);
    mv_context_destroy(ctx);
    mv_context_destroy(other);
    return 0;
}
