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
#include "malvado/malvado.h"

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    MvProcessId root, owned, independent, resolved;
    MvProcessRef ref;
    MvProcessHandle *handle = NULL;
    MvProcessHandleInfo info;
    MvProcessStatus status;
    int64_t priority;
    int killed;
    int ok = mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &root) == MV_OK &&
        mv_process_spawn(ctx, root, 0, idle, NULL, NULL, &owned) == MV_OK &&
        mv_process_spawn_related(ctx, root, 0, 0, idle, NULL, NULL, &independent) == MV_OK &&
        mv_process_ref_create(ctx, root, &ref) == MV_OK &&
        mv_process_ref_resolve(ctx, ref, &resolved) == MV_OK && resolved == root &&
        mv_process_handle_create(ctx, ref, &handle) == MV_OK &&
        mv_process_set_priority(ctx, owned, 10) == MV_OK &&
        mv_process_priority(ctx, owned, &priority) == MV_OK && priority == 10 &&
        mv_process_set_type(ctx, owned, 7) == MV_OK &&
        mv_process_signal(ctx, root, MV_SIGNAL_SLEEP, 1) == MV_OK &&
        mv_process_status(ctx, owned, &status) == MV_OK && status == MV_PROCESS_SLEEPING &&
        mv_process_status(ctx, independent, &status) == MV_OK && status == MV_PROCESS_RUNNING &&
        mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_FREEZE) == MV_OK &&
        mv_process_status(ctx, independent, &status) == MV_OK && status == MV_PROCESS_FROZEN &&
        mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_WAKEUP) == MV_OK &&
        mv_process_kill(ctx, ref, &killed) == MV_OK && killed == 1 &&
        mv_process_count(ctx) == 1 &&
        mv_process_kill(ctx, ref, &killed) == MV_OK && killed == 0;
    mv_context_destroy(ctx);
    ok = ok && mv_process_handle_snapshot(handle, &info) == MV_OK &&
        !info.alive && info.reason == MV_END_KILLED;
    mv_process_handle_destroy(handle);
    if (ok)
        puts("Ownership and creation signals applied; root handle retained");
    return ok ? 0 : 1;
}
