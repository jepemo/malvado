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
#include <stdio.h>
#include <string.h>

typedef struct { int label, calls, destroyed; MvProcessId child; } State;
static int trace[16], trace_count;
static State child = {.label = 3};

static void destroy(void *user) { ((State *)user)->destroyed++; }
static MvStepResult child_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id;
    State *state = user;
    trace[trace_count++] = state->label;
    state->calls++;
    return MV_STEP_YIELD;
}
static MvStepResult parent_step(MvContext *ctx, MvProcessId id, void *user)
{
    State *state = user;
    trace[trace_count++] = state->label;
    state->calls++;
    assert(mv_process_tick(ctx) == MV_BUSY);
    if (!state->child) {
        assert(mv_process_spawn(ctx, id, 100, child_step, destroy, &child, &state->child) == MV_OK);
    }
    return MV_STEP_YIELD;
}

static MvStepResult fail_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    (void)user;
    /* Destruction during a callback must leave the context usable. */
    mv_context_destroy(ctx);
    return MV_STEP_ERROR;
}

int main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    State a = {.label = 1}, b = {.label = 2};
    MvProcessId aid, bid;
    assert(mv_process_spawn(ctx, 0, 0, parent_step, destroy, &a, &aid) == MV_OK);
    assert(mv_process_spawn(ctx, 0, 10, child_step, destroy, &b, &bid) == MV_OK);
    assert(mv_process_set_type(ctx, bid, 42) == MV_OK);
    char bytes[] = "data";
    MvMessage sent = {.type = MV_MESSAGE_BYTES, .value.bytes = {bytes, 4}};
    assert(mv_process_send(ctx, bid, &sent) == MV_OK);
    bytes[0] = 'X';
    MvMessage received;
    assert(mv_process_receive(ctx, bid, &received) == MV_OK);
    assert(received.type == MV_MESSAGE_BYTES && received.value.bytes.size == 4);
    assert(memcmp(received.value.bytes.data, "data", 4) == 0);
    mv_message_release(&received);
    assert(mv_process_receive(ctx, bid, &received) == MV_QUEUE_EMPTY);
    sent = (MvMessage){.type = MV_MESSAGE_INTEGER, .value.integer = 42};
    for (int i = 0; i < 1024; ++i) assert(mv_process_send(ctx, bid, &sent) == MV_OK);
    assert(mv_process_send(ctx, bid, &sent) == MV_QUEUE_FULL);
    size_t count;
    assert(mv_process_message_count(ctx, bid, &count) == MV_OK && count == 1024);
    for (int i = 0; i < 1024; ++i) {
        assert(mv_process_receive(ctx, bid, &received) == MV_OK);
        assert(received.type == MV_MESSAGE_INTEGER && received.value.integer == 42);
        mv_message_release(&received);
    }
    assert(mv_process_tick(ctx) == MV_OK);
    assert(trace_count == 2 && trace[0] == 2 && trace[1] == 1);
    assert(mv_process_count(ctx) == 3);
    MvProcessInfo info[2];
    size_t total;
    assert(mv_process_snapshot(ctx, info, 2, &total) == MV_OK && total == 3);
    assert(info[0].id == aid && info[1].id == bid && info[1].type == 42 && info[1].priority == 10);
    assert(mv_process_signal(ctx, aid, MV_SIGNAL_SLEEP, 1) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(trace_count == 3 && trace[2] == 2);
    assert(mv_process_signal(ctx, aid, MV_SIGNAL_WAKEUP, 1) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(trace_count == 6 && trace[3] == 3 && trace[4] == 2 && trace[5] == 1);
    assert(mv_process_signal(ctx, aid, MV_SIGNAL_KILL, 1) == MV_OK);
    assert(mv_process_count(ctx) == 1);
    assert(mv_process_signal(ctx, aid, MV_SIGNAL_WAKEUP, 1) == MV_NOT_FOUND);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(a.destroyed == 1 && child.destroyed == 1);
    State failed = {0};
    MvProcessId failed_id;
    assert(mv_process_spawn(ctx, 0, 100, fail_step, destroy, &failed, &failed_id) == MV_OK);
    assert(mv_process_tick(ctx) == MV_PROCESS_ERROR);
    assert(failed.destroyed == 1 && mv_process_count(ctx) == 1);
    assert(mv_process_signal(ctx, failed_id, MV_SIGNAL_KILL, 0) == MV_NOT_FOUND);
    mv_context_destroy(ctx);
    assert(b.destroyed == 1);
    puts("C process lifecycle passed");
    return 0;
}
