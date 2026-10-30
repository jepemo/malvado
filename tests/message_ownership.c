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

typedef struct {
    MvContext *ctx;
    int messages_released;
    int process_destroyed;
    int children_destroyed;
    int child_runs;
} State;

static void
release_message(void *user)
{
    State *state = user;
    state->messages_released++;
}

static void
destroy_child(void *user)
{
    State *state = user;
    state->children_destroyed++;
}

static MvStepResult
child_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    State *state = user;
    state->child_runs++;
    return MV_STEP_DONE;
}

static void
destroy_parent(void *user)
{
    State *state = user;
    state->process_destroyed++;
    /* Force growth while the scheduler is running cleanup. */
    for (int i = 0; i < 20; ++i) {
        MvProcessId child;
        assert(mv_process_spawn(state->ctx, 0, 0, child_step, destroy_child, state, &child) == MV_OK);
    }
}

static MvStepResult
parent_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)user;
    assert(mv_process_signal(ctx, id, MV_SIGNAL_KILL, 0) == MV_OK);
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    State state = {.ctx = ctx};
    MvProcessId id;
    assert(mv_process_spawn(ctx, 0, 0, parent_step, destroy_parent, &state, &id) == MV_OK);
    MvMessage message = {
        .type = MV_MESSAGE_OPAQUE,
        .value.opaque = {&state, release_message}
    };
    assert(mv_process_send(ctx, id + 1, &message) == MV_NOT_FOUND);
    assert(state.messages_released == 0);
    assert(mv_process_send(ctx, id, &message) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(state.messages_released == 1 && state.process_destroyed == 1);
    assert(state.child_runs == 0 && mv_process_count(ctx) == 20);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(state.child_runs == 20 && state.children_destroyed == 20);
    assert(mv_process_count(ctx) == 0);
    mv_context_destroy(ctx);
    assert(state.messages_released == 1 && state.process_destroyed == 1);
    return 0;
}
