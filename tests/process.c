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

typedef struct {
    MvContext *ctx;
    MvProcessId creator, owner, independent;
    int destroyed;
} CleanupSpawnState;

static void cleanup_spawn(void *user)
{
    CleanupSpawnState *state = user;
    MvProcessId result = 999;
    assert(mv_process_spawn(state->ctx, state->owner, 0, idle_step, NULL, NULL,
                            &result) == MV_BUSY && result == 999);
    assert(mv_process_spawn_related(state->ctx, state->owner, state->owner, 0, idle_step,
                                    NULL, NULL, &result) == MV_BUSY && result == 999);
    assert(mv_process_spawn_related(state->ctx, state->creator, 0, 0, idle_step,
                                    NULL, NULL, &state->independent) == MV_OK);
    state->destroyed++;
}

static MvStepResult finish_immediately(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_DONE;
}

typedef struct {
    MvContext *ctx;
    MvProcessId victim;
    int destroyed;
} CleanupKillState;

static void cleanup_kill(void *user)
{
    CleanupKillState *state = user;
    assert(mv_process_tick(state->ctx) == MV_BUSY);
    assert(mv_process_signal(state->ctx, state->victim, MV_SIGNAL_KILL, 0) == MV_OK);
    state->destroyed++;
}

static MvStepResult self_kill_step(MvContext *ctx, MvProcessId id, void *user)
{
    State *state = user;
    MvProcessRef ref;
    int killed = 0;
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed == 1);
    state->calls++;
    return MV_STEP_YIELD;
}

typedef struct {
    MvProcessId victim;
    int calls;
} KillerState;

static MvStepResult
kill_before_turn(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    KillerState *state = user;
    ++state->calls;
    assert(mv_process_signal(ctx, state->victim, MV_SIGNAL_KILL, 0) == MV_OK);
    return MV_STEP_DONE;
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
    int64_t unchanged_priority = 123;
    MvProcessStatus unchanged_status = MV_PROCESS_FROZEN;
    assert(mv_process_priority(NULL, bid, &unchanged_priority) == MV_INVALID_ARGUMENT);
    assert(mv_process_priority(ctx, bid, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_priority(ctx, UINT64_MAX, &unchanged_priority) == MV_NOT_FOUND);
    assert(unchanged_priority == 123);
    assert(mv_process_status(NULL, bid, &unchanged_status) == MV_INVALID_ARGUMENT);
    assert(mv_process_status(ctx, bid, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_status(ctx, UINT64_MAX, &unchanged_status) == MV_NOT_FOUND);
    assert(unchanged_status == MV_PROCESS_FROZEN);
    assert(mv_process_set_priority(NULL, bid, 5) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_priority(ctx, UINT64_MAX, 5) == MV_NOT_FOUND);
    assert(mv_process_set_type(NULL, bid, 5) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_type(ctx, UINT64_MAX, 5) == MV_NOT_FOUND);
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
    MvProcessInfo captured[3] = {{.id = 999}};
    MvLineageInfo history[3] = {{.id = 888}};
    size_t processes_needed = 0, history_needed = 0;
    MvInspectionHeader header = {.runtime = {1, 2, 3}, .tick = 99};
    processes_needed = history_needed = 77;
    assert(mv_process_inspection_capture(ctx, NULL, captured, 3, &processes_needed,
                                         history, 3, &history_needed) == MV_INVALID_ARGUMENT);
    assert(processes_needed == 77 && history_needed == 77 && captured[0].id == 999 && history[0].id == 888);
    assert(mv_process_inspection_capture(ctx, &header, captured, 3, &processes_needed,
                                         history, 2, &history_needed) == MV_BUFFER_TOO_SMALL);
    assert(header.tick == 99 && header.runtime.high == 1 && captured[0].id == 999 && history[0].id == 888);
    assert(mv_process_inspection_capture(ctx, &header, captured, 3, &processes_needed,
                                         history, 3, &history_needed) == MV_OK);
    MvRuntimeId runtime;
    assert(mv_context_runtime_id(ctx, &runtime) == MV_OK);
    assert(header.tick == 1 && header.runtime.high == runtime.high &&
           header.runtime.low == runtime.low && header.runtime.sequence == runtime.sequence);
    captured[0].id = 999;
    history[0].id = 888;
    assert(mv_process_inspection_snapshot(ctx, captured, 3, &processes_needed,
                                          history, 2, &history_needed) == MV_BUFFER_TOO_SMALL);
    assert(processes_needed == 3 && history_needed == 3);
    assert(captured[0].id == 999 && history[0].id == 888);
    assert(mv_process_inspection_snapshot(ctx, captured, 3, &processes_needed,
                                          history, 3, &history_needed) == MV_OK);
    assert(processes_needed == 3 && history_needed == 3);
    assert(captured[0].id == aid && captured[1].id == bid &&
           captured[2].id == a.child);
    assert(history[0].id == aid && history[1].id == bid && history[2].id == a.child);
    assert(history[2].creator_id == aid && history[2].alive);
    size_t unchanged_count = 99;
    assert(mv_process_inspection_snapshot(ctx, captured, 3, &unchanged_count,
                                          history, 3, &unchanged_count) == MV_INVALID_ARGUMENT);
    assert(unchanged_count == 99);

    /* Each signal replaces the current live state, including repeated signals. */
    MvProcessStatus signal_status;
    const MvSignal transitions[] = {MV_SIGNAL_SLEEP, MV_SIGNAL_SLEEP, MV_SIGNAL_FREEZE,
                                    MV_SIGNAL_FREEZE, MV_SIGNAL_SLEEP, MV_SIGNAL_WAKEUP,
                                    MV_SIGNAL_WAKEUP};
    const MvProcessStatus expected[] = {MV_PROCESS_SLEEPING, MV_PROCESS_SLEEPING,
        MV_PROCESS_FROZEN, MV_PROCESS_FROZEN, MV_PROCESS_SLEEPING,
        MV_PROCESS_RUNNING, MV_PROCESS_RUNNING};
    for (size_t i = 0; i < sizeof(transitions) / sizeof(transitions[0]); ++i) {
        assert(mv_process_signal(ctx, bid, transitions[i], 0) == MV_OK);
        assert(mv_process_status(ctx, bid, &signal_status) == MV_OK && signal_status == expected[i]);
    }
    assert(mv_process_signal(ctx, bid, (MvSignal)99, 0) == MV_INVALID_ARGUMENT);
    assert(mv_process_status(ctx, bid, &signal_status) == MV_OK && signal_status == MV_PROCESS_RUNNING);

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
    /* Tree signals affect current descendants, not later or unrelated processes. */
    MvContext *tree = mv_context_create(60, 5);
    assert(tree);
    MvProcessId tree_root, tree_child, tree_grandchild, outsider, late_child;
    MvProcessStatus tree_status;
    assert(mv_process_spawn(tree, 0, 0, idle_step, NULL, NULL, &tree_root) == MV_OK);
    assert(mv_process_spawn(tree, tree_root, 0, idle_step, NULL, NULL, &tree_child) == MV_OK);
    assert(mv_process_spawn(tree, tree_child, 0, idle_step, NULL, NULL, &tree_grandchild) == MV_OK);
    assert(mv_process_spawn(tree, 0, 0, idle_step, NULL, NULL, &outsider) == MV_OK);
    assert(mv_process_signal(tree, tree_root, MV_SIGNAL_FREEZE, 1) == MV_OK);
    assert(mv_process_status(tree, tree_root, &tree_status) == MV_OK && tree_status == MV_PROCESS_FROZEN);
    assert(mv_process_status(tree, tree_child, &tree_status) == MV_OK && tree_status == MV_PROCESS_FROZEN);
    assert(mv_process_status(tree, tree_grandchild, &tree_status) == MV_OK && tree_status == MV_PROCESS_FROZEN);
    assert(mv_process_status(tree, outsider, &tree_status) == MV_OK && tree_status == MV_PROCESS_RUNNING);
    assert(mv_process_spawn(tree, tree_root, 0, idle_step, NULL, NULL, &late_child) == MV_OK);
    assert(mv_process_status(tree, late_child, &tree_status) == MV_OK && tree_status == MV_PROCESS_RUNNING);
    assert(mv_process_signal(tree, tree_root, MV_SIGNAL_WAKEUP, 1) == MV_OK);
    assert(mv_process_status(tree, tree_grandchild, &tree_status) == MV_OK && tree_status == MV_PROCESS_RUNNING);
    mv_context_destroy(tree);
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
    assert(mv_process_set_type(relations, independent_id, 17) == MV_OK);
    assert(mv_process_find(relations, 17, &relation_info, 1, &total) == MV_OK &&
           total == 1 && relation_info.id == independent_id);
    assert(mv_process_creation_children(relations, creator_id, 17, &relation_info,
                                        1, &total) == MV_OK && total == 1 &&
           relation_info.id == independent_id);
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
    assert(mv_process_creation_children(relations, creator_id, 0, &relation_info,
                                        1, &total) == MV_OK && total == 0);
    assert(mv_process_creation_children(relations, independent_id, 0, &relation_info,
                                        1, &total) == MV_OK && total == 1 &&
           relation_info.id == leaf_id);
    total = 123;
    assert(mv_process_find(relations, 0, NULL, 1, &total) == MV_INVALID_ARGUMENT && total == 123);
    assert(mv_process_creation_children(relations, UINT64_MAX, 0, &relation_info,
                                        1, &total) == MV_NOT_FOUND && total == 123);
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
    assert(mv_process_spawn(cascade, owner_id, 0, child_step,
                            destroy, &owned, &owned_id) == MV_OK);
    assert(mv_process_signal(cascade, owner_id, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_count(cascade) == 0);
    assert(mv_process_tick(cascade) == MV_OK);
    assert(owner.calls == 0 && owned.calls == 0);
    assert(owner.destroyed == 1 && owned.destroyed == 1);
    mv_context_destroy(cascade);
    MvContext *order = mv_context_create(60, 5);
    assert(order);
    State victim = {0};
    KillerState killer = {0};
    MvProcessId killer_id;
    assert(mv_process_spawn(order, 0, 0, child_step, destroy, &victim, &killer.victim) == MV_OK);
    assert(mv_process_spawn(order, 0, 1, kill_before_turn, NULL, &killer, &killer_id) == MV_OK);
    assert(mv_process_tick(order) == MV_OK);
    assert(killer.calls == 1 && victim.calls == 0 && victim.destroyed == 1);
    mv_context_destroy(order);

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
    MvContext *growth = mv_context_create(60, 5);
    assert(growth);
    MvProcessId growth_root, growth_child;
    assert(mv_process_spawn(growth, 0, 0, idle_step, NULL, NULL, &growth_root) == MV_OK);
    for (int i = 0; i < 100; ++i)
        assert(mv_process_spawn_related(growth, growth_root, 0, 0, idle_step, NULL,
                                        NULL, &growth_child) == MV_OK);
    assert(mv_process_creation_descendants(growth, growth_root, 0, NULL, 0, &total) == MV_OK &&
           total == 100);
    assert(mv_process_signal_creation_tree(growth, growth_root, MV_SIGNAL_KILL) == MV_OK);
    assert(mv_process_tick(growth) == MV_OK);
    assert(mv_process_lineage_snapshot(growth, NULL, 0, &total) == MV_OK && total == 0);
    mv_context_destroy(growth);

    MvContext *ownership = mv_context_create(60, 5);
    assert(ownership);
    MvProcessId ownership_root, ownership_child, ownership_previous;
    assert(mv_process_spawn(ownership, 0, 0, idle_step, NULL, NULL, &ownership_root) == MV_OK);
    ownership_previous = ownership_root;
    for (int i = 0; i < 512; ++i) {
        assert(mv_process_spawn(ownership, ownership_previous, 0, idle_step, NULL, NULL,
                                &ownership_child) == MV_OK);
        ownership_previous = ownership_child;
    }
    assert(mv_process_descendants(ownership, ownership_root, 0, NULL, 0, &total) == MV_OK &&
           total == 512);
    assert(mv_process_signal(ownership, ownership_root, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_count(ownership) == 0 && mv_process_tick(ownership) == MV_OK);
    assert(mv_process_lineage_snapshot(ownership, NULL, 0, &total) == MV_OK && total == 0);
    mv_context_destroy(ownership);

    MvContext *divergent = mv_context_create(60, 5);
    assert(divergent);
    MvProcessId owner_root, creator_root, intermediate, survivor;
    assert(mv_process_spawn(divergent, 0, 0, idle_step, NULL, NULL, &owner_root) == MV_OK);
    assert(mv_process_spawn(divergent, 0, 0, idle_step, NULL, NULL, &creator_root) == MV_OK);
    assert(mv_process_spawn_related(divergent, creator_root, owner_root, 0, idle_step, NULL,
                                    NULL, &intermediate) == MV_OK);
    assert(mv_process_spawn_related(divergent, intermediate, 0, 0, idle_step, NULL,
                                    NULL, &survivor) == MV_OK);
    assert(mv_process_signal(divergent, owner_root, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_tick(divergent) == MV_OK);
    assert(mv_process_creation_descendants(divergent, creator_root, 0, &relation_info,
                                           1, &total) == MV_OK);
    assert(total == 1 && relation_info.id == survivor && relation_info.parent == 0);
    assert(mv_process_descendants(divergent, creator_root, 0, NULL, 0, &total) == MV_OK &&
           total == 0);
    assert(mv_process_signal_creation_tree(divergent, creator_root, MV_SIGNAL_KILL) == MV_OK);
    assert(mv_process_tick(divergent) == MV_OK);
    assert(mv_process_lineage_snapshot(divergent, NULL, 0, &total) == MV_OK && total == 0);
    mv_context_destroy(divergent);

    MvContext *first_runtime = mv_context_create(60, 5);
    MvContext *second_runtime = mv_context_create(60, 5);
    assert(first_runtime && second_runtime);
    MvProcessId first_process, second_process, resolved = 99;
    assert(mv_process_spawn(first_runtime, 0, 0, idle_step, NULL, NULL, &first_process) == MV_OK);
    assert(mv_process_spawn(second_runtime, 0, 0, idle_step, NULL, NULL, &second_process) == MV_OK);
    assert(first_process == second_process);
    MvProcessRef first_ref, second_ref;
    assert(mv_process_ref_create(first_runtime, first_process, &first_ref) == MV_OK);
    assert(mv_process_ref_create(second_runtime, second_process, &second_ref) == MV_OK);
    assert(first_ref.runtime.sequence != second_ref.runtime.sequence);
    assert(mv_process_ref_resolve(second_runtime, first_ref, &resolved) == MV_INVALID_ARGUMENT);
    assert(resolved == 99);
    int killed = 99;
    assert(mv_process_kill(second_runtime, first_ref, &killed) == MV_INVALID_ARGUMENT);
    assert(killed == 99);
    assert(mv_process_kill(first_runtime, first_ref, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_kill(first_runtime, first_ref, &killed) == MV_OK && killed == 1);
    assert(mv_process_kill(first_runtime, first_ref, &killed) == MV_OK && killed == 0);
    assert(mv_process_tick(first_runtime) == MV_OK);
    assert(mv_process_kill(first_runtime, first_ref, &killed) == MV_OK && killed == 0);
    assert(mv_process_ref_resolve(first_runtime, first_ref, &resolved) == MV_OK &&
           resolved == first_process);
    mv_context_destroy(first_runtime);
    MvContext *later_runtime = mv_context_create(60, 5);
    assert(later_runtime);
    assert(mv_process_ref_resolve(later_runtime, first_ref, &resolved) == MV_INVALID_ARGUMENT);
    assert(mv_process_ref_resolve(later_runtime, second_ref, &resolved) == MV_INVALID_ARGUMENT);
    mv_context_destroy(later_runtime);
    mv_context_destroy(second_runtime);

    MvContext *self_context = mv_context_create(60, 5);
    State self_state = {0};
    MvProcessId self_id;
    assert(self_context);
    assert(mv_process_spawn(self_context, 0, 0, self_kill_step, destroy,
                            &self_state, &self_id) == MV_OK);
    assert(mv_process_tick(self_context) == MV_OK);
    assert(self_state.calls == 1 && self_state.destroyed == 1);
    assert(mv_process_tick(self_context) == MV_OK);
    assert(self_state.calls == 1 && self_state.destroyed == 1);
    mv_context_destroy(self_context);

    MvContext *cleanup_context = mv_context_create(60, 5);
    assert(cleanup_context);
    CleanupSpawnState cleanup_state = {.ctx = cleanup_context};
    assert(mv_process_spawn(cleanup_context, 0, 0, idle_step, NULL, NULL,
                            &cleanup_state.owner) == MV_OK);
    assert(mv_process_spawn(cleanup_context, 0, 0, finish_immediately, cleanup_spawn,
                            &cleanup_state, &cleanup_state.creator) == MV_OK);
    assert(mv_process_tick(cleanup_context) == MV_OK);
    assert(cleanup_state.destroyed == 1 && cleanup_state.independent != 0);
    MvProcessInfo descendants[1];
    size_t descendants_count = 0;
    assert(mv_process_creation_descendants(cleanup_context, cleanup_state.creator, 0,
                                           descendants, 1, &descendants_count) == MV_OK);
    assert(descendants_count == 1 && descendants[0].id == cleanup_state.independent);
    assert(mv_process_tick(cleanup_context) == MV_OK && cleanup_state.destroyed == 1);
    mv_context_destroy(cleanup_context);

    {
        MvContext *reentrant = mv_context_create(60, 5);
        assert(reentrant);
        State victim = {0}, victim_child = {0};
        CleanupKillState killer = {.ctx = reentrant};
        MvProcessId killer_id, victim_child_id;
        assert(mv_process_spawn(reentrant, 0, 0, finish_immediately, cleanup_kill,
                                &killer, &killer_id) == MV_OK);
        assert(mv_process_spawn(reentrant, 0, 0, idle_step, destroy, &victim,
                                &killer.victim) == MV_OK);
        assert(mv_process_spawn(reentrant, killer.victim, 0, idle_step, destroy, &victim_child,
                                &victim_child_id) == MV_OK);
        assert(mv_process_tick(reentrant) == MV_OK);
        assert(killer.destroyed == 1 && victim.destroyed == 1 && victim_child.destroyed == 1);
        assert(mv_process_count(reentrant) == 0);
        assert(mv_process_tick(reentrant) == MV_OK);
        mv_context_destroy(reentrant);
        assert(killer.destroyed == 1 && victim.destroyed == 1 && victim_child.destroyed == 1);
    }
    puts("C process lifecycle passed");
    return 0;
}
