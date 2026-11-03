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

static MvStepResult idle_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
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
    State leaf = {0};
    MvProcessId leaf_id;
    assert(mv_process_spawn_related(relations, independent_id, 0, 0, child_step,
                                    destroy, &leaf, &leaf_id) == MV_OK);
    assert(mv_process_signal(relations, independent_id, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_tick(relations) == MV_OK);
    MvLineageInfo lineage[3];
    assert(mv_process_lineage_snapshot(relations, lineage, 3, &total) == MV_OK && total == 3);
    assert(lineage[0].id == creator_id && !lineage[0].alive && lineage[0].creator_id == 0);
    assert(lineage[1].id == independent_id && !lineage[1].alive &&
           lineage[1].creator_id == creator_id);
    assert(lineage[2].id == leaf_id && lineage[2].alive &&
           lineage[2].creator_id == independent_id);
    assert(mv_process_creation_descendants(relations, creator_id, 0, &relation_info,
                                           1, &total) == MV_OK);
    assert(total == 1 && relation_info.id == leaf_id);
    assert(mv_process_signal_creation_tree(relations, creator_id, MV_SIGNAL_FREEZE) == MV_OK);
    MvProcessStatus leaf_status;
    assert(mv_process_status(relations, leaf_id, &leaf_status) == MV_OK &&
           leaf_status == MV_PROCESS_FROZEN);
    assert(mv_process_signal_creation_tree(relations, creator_id, (MvSignal)99) == MV_INVALID_ARGUMENT);
    assert(mv_process_status(relations, leaf_id, &leaf_status) == MV_OK &&
           leaf_status == MV_PROCESS_FROZEN);
    assert(mv_process_signal_creation_tree(relations, creator_id, MV_SIGNAL_KILL) == MV_OK);
    assert(mv_process_count(relations) == 0);
    assert(mv_process_tick(relations) == MV_OK);
    assert(mv_process_creation_descendants(relations, creator_id, 0, NULL, 0, &total) == MV_OK &&
           total == 0);
    assert(mv_process_lineage_snapshot(relations, NULL, 0, &total) == MV_OK && total == 0);
    mv_context_destroy(relations);
    assert(creator.destroyed == 1 && independent.destroyed == 1 && leaf.destroyed == 1);
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
    assert(mv_process_set_type(cascade, nested_id, 42) == MV_OK);
    assert(mv_process_signal(cascade, owned_id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    MvProcessInfo owned_info[2];
    assert(mv_process_children(cascade, owner_id, 0, owned_info, 2, &total) == MV_OK);
    assert(total == 1 && owned_info[0].id == owned_id && owned_info[0].attached &&
           owned_info[0].status == MV_PROCESS_FROZEN);
    assert(mv_process_descendants(cascade, owner_id, 0, owned_info, 1, &total) == MV_OK);
    assert(total == 2 && owned_info[0].id == owned_id);
    assert(mv_process_descendants(cascade, owner_id, 42, owned_info, 2, &total) == MV_OK);
    assert(total == 1 && owned_info[0].id == nested_id);
    total = 123;
    assert(mv_process_children(cascade, UINT64_MAX, 0, owned_info, 2, &total) == MV_NOT_FOUND);
    assert(total == 123);
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
    MvContext *deep = mv_context_create(60, 5);
    assert(deep);
    MvProcessId root_id, current_id, next_id;
    assert(mv_process_spawn_related(deep, 0, 0, 0, idle_step, NULL, NULL, &root_id) == MV_OK);
    current_id = root_id;
    for (int i = 0; i < 512; ++i) {
        assert(mv_process_spawn_related(deep, current_id, 0, 0, idle_step, NULL,
                                        NULL, &next_id) == MV_OK);
        assert(mv_process_signal(deep, current_id, MV_SIGNAL_KILL, 0) == MV_OK);
        assert(mv_process_tick(deep) == MV_OK);
        current_id = next_id;
    }
    assert(mv_process_lineage_snapshot(deep, NULL, 0, &total) == MV_OK && total == 513);
    assert(mv_process_creation_descendants(deep, root_id, 0, NULL, 0, &total) == MV_OK &&
           total == 1);
    assert(mv_process_signal_creation_tree(deep, root_id, MV_SIGNAL_KILL) == MV_OK);
    assert(mv_process_tick(deep) == MV_OK);
    assert(mv_process_lineage_snapshot(deep, NULL, 0, &total) == MV_OK && total == 0);
    mv_context_destroy(deep);
    puts("C process lifecycle passed");
    return 0;
}
