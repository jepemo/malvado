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

typedef struct { int created, steps, destroyed; } State;

static MvStepResult
step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    State *state = user;
    ++state->steps;
    return MV_STEP_YIELD;
}

static void
destroy(void *user)
{
    State *state = user;
    ++state->destroyed;
}

static void
inspect(void *user)
{
    State *state = user;
    if (state->created) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessInfo items[2];
    size_t total;
    assert(mv_process_snapshot(ctx, items, 2, &total) == MV_OK && total == 2);
    MvProcessId worker = items[1].id, root, created, owned;
    assert(mv_process_spawn_related(ctx, 0, 0, 0, step, destroy, state, &root) == MV_OK);
    assert(mv_process_spawn_related(ctx, root, worker, 0, step, destroy, state, &created) == MV_OK);
    assert(mv_process_spawn_related(ctx, worker, root, 0, step, destroy, state, &owned) == MV_OK);
    assert(mv_process_set_type(ctx, root, 1) == MV_OK);
    assert(mv_process_set_type(ctx, created, 1) == MV_OK);
    assert(mv_process_set_type(ctx, owned, 1) == MV_OK);
    state->created = 1;
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
    assert(result == 0 && state.created && state.steps == 0 && state.destroyed == 3);
    return 0;
}
