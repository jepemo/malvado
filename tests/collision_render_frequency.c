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
#include "malvado/collision.h"
#include "runner/runtime.h"

typedef struct {
    int frames_per_tick, frames, count, hits[4];
    int64_t last_tick, ticks[4];
    MvProcessRef target, player;
} Trace;

static void
observe_tick(void *user)
{
    Trace *trace = user;
    MalvadoProcessInfo info[2];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(info, 2, &count, &tick));
    if (tick == trace->last_tick)
        return;
    trace->last_tick = tick;
    if (!trace->count) {
        MvContext *ctx = malvado_debug_context();
        assert(count == 2 && malvado_debug_pause(1));
        assert(mv_process_ref_create(ctx, info[0].id, &trace->target) == MV_OK);
        assert(mv_process_ref_create(ctx, info[1].id, &trace->player) == MV_OK);
    }
    if (trace->count < 4) {
        int hit = -1;
        assert(mv_process_collider_test(malvado_debug_context(), trace->target,
                                        trace->player, &hit) == MV_OK);
        trace->ticks[trace->count] = tick;
        trace->hits[trace->count++] = hit;
    }
}

static void
present_frame(void *user)
{
    Trace *trace = user;
    if (++trace->frames % trace->frames_per_tick == 0)
        assert(malvado_debug_step());
}

static Trace
run(const char *script, int frames_per_tick)
{
    Trace trace = {.frames_per_tick = frames_per_tick};
    char *args[] = {"malvado-run", (char *)script};
    malvado_debug_set_hook(observe_tick, &trace);
    malvado_debug_set_frame_hook(present_frame, &trace);
    assert(malvado_run(2, args) == 0);
    malvado_debug_set_frame_hook(NULL, NULL);
    malvado_debug_set_hook(NULL, NULL);
    assert(trace.count == 4 && trace.frames >= 4 * frames_per_tick);
    return trace;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    Trace one = run(argv[1], 1);
    Trace four = run(argv[1], 4);
    const int expected[] = {0, 1, 1, 0};
    for (int i = 0; i < 4; ++i)
        assert(one.hits[i] == expected[i] && four.hits[i] == expected[i] &&
               one.ticks[i] == four.ticks[i] && one.ticks[i] == i + 1);
    return 0;
}
