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

#include "runner/runtime.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

typedef struct {
    int phase;
    int native_runs;
    int native_destroyed;
} State;

static void
native_destroy(void *user)
{
    State *state = user;
    state->native_destroyed++;
}

static MvStepResult
native_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    State *state = user;
    state->native_runs++;
    if (state->native_runs == 1) {
        assert(mv_process_signal(ctx, 1, MV_SIGNAL_SLEEP, 1) == MV_OK);
        return MV_STEP_YIELD;
    }
    assert(state->native_runs == 2);
    assert(mv_process_signal(ctx, 1, MV_SIGNAL_KILL, 1) == MV_OK);
    return MV_STEP_YIELD;
}

static void
inspect(void *user)
{
    State *state = user;
    MvContext *ctx = malvado_debug_context();
    MvProcessInfo info[3];
    size_t count;
    assert(mv_process_snapshot(ctx, info, 3, &count) == MV_OK);
    if (state->phase == 0) {
        assert(count == 2 && info[0].id == 1 && info[1].parent == 1);
        MvProcessId id;
        assert(mv_process_spawn(ctx, 2, 10, native_step, native_destroy, state, &id) == MV_OK);
        assert(id == 3);
    } else if (state->phase == 1) {
        assert(count == 3 && state->native_runs == 1);
        for (size_t i = 0; i < count; ++i)
            assert(info[i].status == MV_PROCESS_SLEEPING);
        assert(mv_process_signal(ctx, 1, MV_SIGNAL_WAKEUP, 1) == MV_OK);
    } else if (state->phase == 2) {
        assert(count == 0 && state->native_runs == 2 && state->native_destroyed == 1);
    }
    state->phase++;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    State state = {0};
    malvado_debug_set_hook(inspect, &state);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    int result = malvado_run(3, args);
    malvado_debug_set_hook(NULL, NULL);
    assert(result == 0 && state.phase == 3 && state.native_destroyed == 1);
    return 0;
}
