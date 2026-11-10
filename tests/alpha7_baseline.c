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
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

typedef struct {
    unsigned tick, parent_calls, child_calls, slow_calls, drops;
    MvCadence cadence;
} Trace;

static void
closed(void *user)
{
    ((Trace *)user)->drops++;
}

static MvStepResult
child(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    Trace *trace = user;
    printf("%u child %u\n", trace->tick, 7 + trace->child_calls++);
    return trace->child_calls == 2 ? MV_STEP_DONE : MV_STEP_YIELD;
}

static MvStepResult
parent(MvContext *ctx, MvProcessId id, void *user)
{
    Trace *trace = user;
    printf("%u parent %u\n", trace->tick, 40 + trace->parent_calls++);
    if (trace->parent_calls == 1) {
        MvProcessId spawned;
        assert(mv_process_spawn_related(ctx, id, 0, 0, child, closed, trace,
                                        &spawned) == MV_OK);
        return MV_STEP_YIELD;
    }
    return MV_STEP_DONE;
}

static MvStepResult
slow(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    Trace *trace = user;
    if (trace->slow_calls && !mv_cadence_step(&trace->cadence, 15))
        return MV_STEP_YIELD;
    printf("%u slow %u\n", trace->tick, ++trace->slow_calls);
    return trace->slow_calls == 3 ? MV_STEP_DONE : MV_STEP_YIELD;
}

int
main(void)
{
    Trace trace = {0};
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    MvProcessId id;
    assert(mv_process_spawn(ctx, 0, 0, parent, closed, &trace, &id) == MV_OK);
    assert(mv_process_spawn(ctx, 0, 0, slow, closed, &trace, &id) == MV_OK);
    while (mv_process_count(ctx) && trace.tick < 10) {
        ++trace.tick;
        assert(mv_process_tick(ctx) == MV_OK);
    }
    assert(trace.tick == 9 && trace.drops == 3);
    mv_context_destroy(ctx);
    assert(trace.drops == 3);
    printf("closed %u\n", trace.drops);
    return 0;
}
