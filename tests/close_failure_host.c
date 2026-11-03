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
    int attached;
    int destroyed;
} State;

static void
resource_destroy(void *user)
{
    State *state = user;
    ++state->destroyed;
}

static void
inspect(void *user)
{
    State *state = user;
    if (state->attached) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessInfo processes[4];
    size_t total;
    assert(mv_process_snapshot(ctx, processes, 4, &total) == MV_OK && total == 2);
    MvResource *resource;
    assert(mv_resource_create(ctx, processes[1].id, state, resource_destroy, &resource) == MV_OK);
    state->attached = 1;
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
    assert(result != 0 && state.attached && state.destroyed == 1);
    return 0;
}
