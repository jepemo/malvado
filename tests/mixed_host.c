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
#include <string.h>

typedef struct { int created, calls, destroyed; } State;

static void
destroy(void *user)
{
    ((State *)user)->destroyed++;
}

static MvStepResult
native_step(MvContext *ctx, MvProcessId id, void *user)
{
    State *state = user;
    if (!state->calls++) {
        assert(mv_process_signal(ctx, 1, MV_SIGNAL_FREEZE, 0) == MV_OK);
        assert(mv_process_signal(ctx, 1, MV_SIGNAL_WAKEUP, 0) == MV_OK);
        MvMessage message = {.type = MV_MESSAGE_INTEGER, .value.integer = 7};
        assert(mv_process_send(ctx, 1, &message) == MV_OK);
        return MV_STEP_YIELD;
    }
    MvMessage message;
    assert(mv_process_receive(ctx, id, &message) == MV_OK);
    assert(message.type == MV_MESSAGE_BYTES && message.value.bytes.size == 9);
    assert(memcmp(message.value.bytes.data, "ping\0data", 9) == 0);
    mv_message_release(&message);
    message = (MvMessage){.type = MV_MESSAGE_BYTES, .value.bytes = {(void *)"pong", 4}};
    assert(mv_process_send(ctx, 1, &message) == MV_OK);
    return MV_STEP_DONE;
}

static void
hook(void *user)
{
    State *state = user;
    if (state->created) return;
    state->created = 1;
    MvProcessId id;
    assert(mv_process_spawn(malvado_debug_context(), 1, 10, native_step, destroy, state, &id) == MV_OK);
    assert(id == 2);
    assert(mv_process_set_type(malvado_debug_context(), id, 1) == MV_OK);
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
    assert(result == 0 && state.calls == 2 && state.destroyed == 1);
    return 0;
}
