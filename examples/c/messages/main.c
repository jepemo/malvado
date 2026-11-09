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
#include <stdio.h>
#include <string.h>
#include "malvado/malvado.h"

typedef struct { int received; } State;

static MvStepResult
receive_message(MvContext *ctx, MvProcessId id, void *user)
{
    State *state = user;
    MvMessage message;
    if (mv_process_receive(ctx, id, &message) != MV_OK)
        return MV_STEP_ERROR;
    if (message.type == MV_MESSAGE_BYTES && message.value.bytes.size == 4 &&
        memcmp(message.value.bytes.data, "ping", 4) == 0) {
        puts("Received ping");
        state->received = 1;
    }
    mv_message_release(&message);
    return state->received ? MV_STEP_DONE : MV_STEP_ERROR;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    State state = {0};
    MvProcessId recipient;
    char payload[] = "ping";
    MvMessage message = {.type = MV_MESSAGE_BYTES, .value.bytes = {payload, 4}};
    size_t count = 0;
    int ok = mv_process_spawn(ctx, 0, 0, receive_message, NULL, &state, &recipient) == MV_OK &&
             mv_process_send(ctx, recipient, &message) == MV_OK &&
             mv_process_message_count(ctx, recipient, &count) == MV_OK && count == 1;
    payload[0] = 'P'; /* Sending bytes makes a copy. */
    if (ok)
        ok = mv_process_tick(ctx) == MV_OK && state.received;
    mv_context_destroy(ctx);
    return ok ? 0 : 1;
}
