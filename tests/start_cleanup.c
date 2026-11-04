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
    int created;
    int destroyed;
} State;

static void
destroy_resource(void *user)
{
    State *state = user;
    state->destroyed++;
}

static void
create_resource(void *user)
{
    State *state = user;
    if (state->created)
        return;
    MvResource *resource = NULL;
    assert(mv_resource_create(malvado_debug_context(), NULL, state, destroy_resource, &resource) == MV_OK);
    state->created++;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    for (int i = 0; i < 2; i++) {
        State state = {0};
        malvado_debug_set_hook(create_resource, &state);
        assert(malvado_run(3, args) == 1);
        malvado_debug_set_hook(NULL, NULL);
        assert(state.created == 1 && state.destroyed == 1);
        assert(malvado_debug_context() == NULL);
    }
    return 0;
}
