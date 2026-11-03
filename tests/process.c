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

static MvStepResult finish_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    State *state = user;
    state->calls++;
    return MV_STEP_DONE;
}

typedef struct {
    MvContext *ctx;
    MvProcessId other;
    int destroyed;
} ShutdownState;

static void shutdown_destroy(void *user)
{
    ShutdownState *state = user;
    MvProcessStatus status;
    MvProcessId id = 0;
    assert(mv_process_status(state->ctx, state->other, &status) == MV_NOT_FOUND);
    assert(mv_process_spawn(state->ctx, 0, 0, child_step, NULL, NULL, &id) == MV_BUSY);
    state->destroyed++;
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
    MvContext *relations = mv_context_create(60, 5);
    assert(relations);
    State creator = {0}, independent = {0};
    MvProcessId creator_id, independent_id = 0;
    assert(mv_process_spawn(relations, 0, 0, child_step, destroy, &creator,
                            &creator_id) == MV_OK);
    assert(mv_process_spawn_related(relations, creator_id, 0, 0, child_step,
                                    destroy, &independent, &independent_id) == MV_OK);
    MvProcessId unchanged = 99;
    assert(mv_process_spawn_related(relations, UINT64_MAX, 0, 0, child_step,
                                    destroy, &independent, &unchanged) == MV_NOT_FOUND);
    assert(unchanged == 99);
    assert(mv_process_signal(relations, creator_id, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_tick(relations) == MV_OK);
    MvProcessInfo relation_info;
    assert(mv_process_snapshot(relations, &relation_info, 1, &total) == MV_OK);
    assert(total == 1 && relation_info.id == independent_id);
    assert(relation_info.parent == 0 && relation_info.creator_id == creator_id);
    mv_context_destroy(relations);
    assert(creator.destroyed == 1 && independent.destroyed == 1);
    MvContext *cascade = mv_context_create(60, 5);
    assert(cascade);
    State owner = {0}, owned = {0}, nested = {0}, free_child = {0};
    MvProcessId owner_id, owned_id, nested_id, free_id;
    assert(mv_process_spawn_related(cascade, 0, 0, 10, finish_step, destroy,
                                    &owner, &owner_id) == MV_OK);
    assert(mv_process_spawn_related(cascade, owner_id, owner_id, 0, child_step,
                                    destroy, &owned, &owned_id) == MV_OK);
    assert(mv_process_spawn_related(cascade, owned_id, owned_id, 0, child_step,
                                    destroy, &nested, &nested_id) == MV_OK);
    assert(mv_process_spawn_related(cascade, owner_id, 0, 0, child_step,
                                    destroy, &free_child, &free_id) == MV_OK);
    assert(mv_process_tick(cascade) == MV_OK);
    assert(owner.calls == 1 && owned.calls == 0 && nested.calls == 0);
    assert(owner.destroyed == 1 && owned.destroyed == 1 && nested.destroyed == 1);
    assert(free_child.calls == 1 && free_child.destroyed == 0);
    assert(mv_process_count(cascade) == 1);
    mv_context_destroy(cascade);
    assert(free_child.destroyed == 1);
    cascade = mv_context_create(60, 5);
    assert(cascade);
    owner = (State){0};
    owned = (State){0};
    assert(mv_process_spawn_related(cascade, 0, 0, 0, child_step, destroy,
                                    &owner, &owner_id) == MV_OK);
    assert(mv_process_spawn_related(cascade, owner_id, owner_id, 0, child_step,
                                    destroy, &owned, &owned_id) == MV_OK);
    assert(mv_process_signal(cascade, owner_id, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_count(cascade) == 0);
    assert(mv_process_tick(cascade) == MV_OK);
    assert(owner.calls == 0 && owned.calls == 0);
    assert(owner.destroyed == 1 && owned.destroyed == 1);
    mv_context_destroy(cascade);
    MvContext *shutdown = mv_context_create(60, 5);
    assert(shutdown);
    ShutdownState first = {.ctx = shutdown}, second = {.ctx = shutdown};
    MvProcessId first_id, second_id;
    assert(mv_process_spawn(shutdown, 0, 0, child_step, shutdown_destroy,
                            &first, &first_id) == MV_OK);
    assert(mv_process_spawn(shutdown, 0, 0, child_step, shutdown_destroy,
                            &second, &second_id) == MV_OK);
    first.other = second_id;
    second.other = first_id;
    mv_context_destroy(shutdown);
    assert(first.destroyed == 1 && second.destroyed == 1);
    puts("C process lifecycle passed");
    return 0;
}
