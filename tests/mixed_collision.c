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
#include "malvado/collision.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

typedef struct { int created, calls; MvProcessRef player, native; } State;

static MvStepResult
native_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    State *state = user;
    int hit;
    assert(mv_process_collider_test(ctx, state->player, state->native, &hit) == MV_OK);
    if (state->calls++ == 0) {
        assert(hit);
        return MV_STEP_YIELD;
    }
    assert(!hit);
    return MV_STEP_DONE;
}

static void
hook(void *user)
{
    State *state = user;
    if (state->created) return;
    state->created = 1;
    MvContext *ctx = malvado_debug_context();
    MvProcessId id;
    assert(mv_process_ref_create(ctx, 1, &state->player) == MV_OK);
    assert(mv_process_spawn(ctx, 1, 10, native_step, NULL, state, &id) == MV_OK && id == 2);
    assert(mv_process_ref_create(ctx, id, &state->native) == MV_OK);
    assert(mv_process_set_type(ctx, id, 1) == MV_OK);
    MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = 5};
    assert(mv_process_collider_set(ctx, state->native, circle, (MvCollisionTransform){3, 0, 0, 1}) == MV_OK);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    State state = {0};
    malvado_debug_set_hook(hook, &state);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    int result = malvado_run(3, args);
    malvado_debug_set_hook(NULL, NULL);
    assert(result == 0 && state.calls == 2);
    return 0;
}
