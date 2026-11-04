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
#include "runner/runtime.h"

typedef struct {
    int run, created, destroyed;
    MvProcessRef previous;
} State;

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

static void
destroy(void *user)
{
    State *state = user;
    state->destroyed++;
}

static void
inspect(void *user)
{
    State *state = user;
    if (state->created) return;
    MvContext *ctx = malvado_debug_context();
    if (state->run) {
        MvProcessId unchanged = 999;
        assert(mv_process_ref_resolve(ctx, state->previous, &unchanged) == MV_INVALID_ARGUMENT);
        assert(unchanged == 999);
    }
    MvProcessId id;
    assert(mv_process_spawn(ctx, 0, 0, idle, destroy, state, &id) == MV_OK);
    assert(mv_process_set_type(ctx, id, 1) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &state->previous) == MV_OK);
    state->created = 1;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    State state = {0};
    malvado_debug_set_hook(inspect, &state);
    for (state.run = 0; state.run < 2; state.run++) {
        state.created = 0;
        assert(malvado_run(3, args) == 1);
        assert(state.created && state.destroyed == state.run + 1);
    }
    malvado_debug_set_hook(NULL, NULL);
    return 0;
}
