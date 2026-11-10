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

typedef struct { unsigned calls; MvProcessId attached, detached; } State;
static MvStepResult wait(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static MvStepResult root(MvContext *ctx, MvProcessId id, void *user)
{
    State *s = user;
    if (s->calls++)
        return MV_STEP_DONE;
    assert(mv_process_spawn_related(ctx, id, id, 0, wait, NULL, NULL, &s->attached) == MV_OK);
    assert(mv_process_spawn_related(ctx, id, 0, 0, wait, NULL, NULL, &s->detached) == MV_OK);
    return MV_STEP_YIELD;
}
static MvProcessHandle *handle(MvContext *ctx, MvProcessId id)
{
    MvProcessRef ref;
    MvProcessHandle *out;
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    assert(mv_process_handle_create(ctx, ref, &out) == MV_OK);
    return out;
}
int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    State s = {0};
    MvProcessId id;
    assert(mv_process_spawn(ctx, 0, 0, root, NULL, &s, &id) == MV_OK);
    MvProcessHandle *parent = handle(ctx, id);
    assert(mv_process_tick(ctx) == MV_OK);
    MvProcessHandle *attached = handle(ctx, s.attached);
    size_t owned, created;
    assert(mv_process_children(ctx, id, 0, NULL, 0, &owned) == MV_OK);
    assert(mv_process_creation_children(ctx, id, 0, NULL, 0, &created) == MV_OK);
    printf("owned=%zu created=%zu\n", owned, created);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 1) == MV_OK);
    MvProcessStatus a, b;
    assert(mv_process_status(ctx, s.attached, &a) == MV_OK && a == MV_PROCESS_FROZEN);
    assert(mv_process_status(ctx, s.detached, &b) == MV_OK && b == MV_PROCESS_RUNNING);
    puts("ownership: frozen running");
    assert(mv_process_signal_creation_tree(ctx, id, MV_SIGNAL_WAKEUP) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    MvProcessHandleInfo p, child;
    assert(mv_process_handle_snapshot(parent, &p) == MV_OK && !p.alive && p.reason == MV_END_RETURNED);
    assert(mv_process_handle_snapshot(attached, &child) == MV_OK && !child.alive && child.reason == MV_END_KILLED);
    puts("parent: returned attached: killed");
    assert(mv_process_creation_descendants(ctx, id, 0, NULL, 0, &created) == MV_OK);
    printf("dead creator: %zu descendant\n", created);
    assert(mv_process_signal_creation_tree(ctx, id, MV_SIGNAL_FREEZE) == MV_OK);
    assert(mv_process_status(ctx, s.detached, &b) == MV_OK && b == MV_PROCESS_FROZEN);
    puts("creation: frozen");
    assert(mv_process_signal_creation_tree(ctx, id, MV_SIGNAL_KILL) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    printf("remaining=%zu\n", mv_process_count(ctx));
    mv_process_handle_destroy(parent);
    mv_process_handle_destroy(attached);
    mv_context_destroy(ctx);
    return 0;
}
