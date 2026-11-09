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


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "malvado/malvado.h"

static int fail_allocation;

void *test_handle_calloc(size_t count, size_t size)
{
    if (fail_allocation) return NULL;
    return calloc(count, size);
}

static void
count_resource_destroy(void *data)
{
    int *count = data;
    ++*count;
}

static MvStepResult
step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    return *(MvStepResult *)user;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    MvContext *other = mv_context_create(60, 5);
    assert(ctx && other);
    MvStepResult waiting = MV_STEP_YIELD, done = MV_STEP_DONE, error = MV_STEP_ERROR;
    MvProcessId owner, child, returned, failed;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, &waiting, &owner) == MV_OK);
    assert(mv_process_spawn_related(ctx, owner, owner, 7, step, NULL, &waiting, &child) == MV_OK);
    assert(mv_process_set_type(ctx, child, 42) == MV_OK);
    MvProcessRef owner_ref, child_ref;
    assert(mv_process_ref_create(ctx, owner, &owner_ref) == MV_OK);
    assert(mv_process_ref_create(ctx, child, &child_ref) == MV_OK);
    int resource_destroyed = 0;
    MvResource *resource = NULL;
    fail_allocation = 1;
    assert(mv_resource_create(ctx, &owner_ref, &resource_destroyed,
                              count_resource_destroy, &resource) == MV_OUT_OF_MEMORY);
    fail_allocation = 0;
    assert(resource == NULL && resource_destroyed == 0);
    assert(mv_resource_create(ctx, &owner_ref, &resource_destroyed,
                              count_resource_destroy, &resource) == MV_OK);
    MvProcessHandle *handle = NULL, *owner_handle = NULL;
    MvProcessHandle *unchanged = (MvProcessHandle *)&waiting;
    assert(mv_process_handle_create(NULL, child_ref, &unchanged) == MV_INVALID_ARGUMENT);
    assert(mv_process_handle_create(ctx, child_ref, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_handle_create(other, child_ref, &unchanged) == MV_INVALID_ARGUMENT);
    assert(unchanged == (MvProcessHandle *)&waiting);
    fail_allocation = 1;
    assert(mv_process_handle_create(ctx, child_ref, &unchanged) == MV_OUT_OF_MEMORY);
    fail_allocation = 0;
    assert(unchanged == (MvProcessHandle *)&waiting);
    assert(mv_process_handle_create(ctx, child_ref, &handle) == MV_OK);
    assert(mv_process_handle_create(ctx, owner_ref, &owner_handle) == MV_OK);
    MvProcessHandleInfo info;
    memset(&info, 0x5a, sizeof(info));
    MvProcessHandleInfo before = info;
    assert(mv_process_handle_snapshot(NULL, &info) == MV_INVALID_ARGUMENT);
    assert(memcmp(&info, &before, sizeof(info)) == 0);
    assert(mv_process_handle_snapshot(handle, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK && info.alive);
    assert(info.process.id == child && info.process.parent == owner && info.process.creator_id == owner);
    assert(info.process.type == 42 && info.process.priority == 7 && info.process.status == MV_PROCESS_RUNNING);
    assert(mv_process_signal(ctx, child, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK && info.process.status == MV_PROCESS_FROZEN);
    int killed;
    assert(mv_process_kill(ctx, owner_ref, &killed) == MV_OK && killed == 1);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK && !info.alive && info.reason == MV_END_KILLED);
    assert(info.process.id == 0 && info.reference.id == child);
    assert(mv_process_handle_create(ctx, child_ref, &unchanged) == MV_NOT_FOUND);
    assert(unchanged == (MvProcessHandle *)&waiting);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(resource_destroyed == 1);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK && !info.alive && info.reason == MV_END_KILLED);
    MvProcessId last_id = child;
    for (int i = 0; i < 32; ++i) {
        MvProcessId next;
        assert(mv_process_spawn(ctx, 0, 0, step, NULL, &done, &next) == MV_OK);
        assert(next > last_id);
        assert(mv_process_tick(ctx) == MV_OK);
        MvProcessId resolved = 0;
        assert(mv_process_ref_resolve(ctx, child_ref, &resolved) == MV_OK && resolved == child);
        assert(mv_process_handle_snapshot(handle, &info) == MV_OK && !info.alive &&
               info.reference.id == child && info.reason == MV_END_KILLED);
        last_id = next;
    }
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, &done, &returned) == MV_OK);
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, &error, &failed) == MV_OK);
    MvProcessRef returned_ref, failed_ref;
    MvProcessHandle *returned_handle, *failed_handle;
    assert(mv_process_ref_create(ctx, returned, &returned_ref) == MV_OK);
    assert(mv_process_ref_create(ctx, failed, &failed_ref) == MV_OK);
    assert(mv_process_handle_create(ctx, returned_ref, &returned_handle) == MV_OK);
    assert(mv_process_handle_create(ctx, failed_ref, &failed_handle) == MV_OK);
    assert(mv_process_tick(ctx) == MV_PROCESS_ERROR);
    assert(mv_process_handle_snapshot(returned_handle, &info) == MV_OK && !info.alive && info.reason == MV_END_RETURNED);
    assert(mv_process_handle_snapshot(failed_handle, &info) == MV_OK && !info.alive && info.reason == MV_END_ERROR);
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, &waiting, &owner) == MV_OK);
    assert(mv_process_ref_create(ctx, owner, &owner_ref) == MV_OK);
    MvProcessHandle *shutdown_handle;
    assert(mv_process_handle_create(ctx, owner_ref, &shutdown_handle) == MV_OK);
    mv_context_destroy(ctx);
    assert(mv_process_handle_snapshot(shutdown_handle, &info) == MV_OK && !info.alive && info.reason == MV_END_KILLED);
    assert(mv_process_handle_snapshot(returned_handle, &info) == MV_OK && info.reason == MV_END_RETURNED);
    MvProcessId result = 999;
    assert(mv_process_ref_resolve(other, child_ref, &result) == MV_INVALID_ARGUMENT && result == 999);
    MvContext *later = mv_context_create(60, 5);
    assert(later);
    assert(mv_process_spawn(later, 0, 0, step, NULL, &waiting, &owner) == MV_OK);
    assert(mv_process_spawn(later, 0, 0, step, NULL, &waiting, &child) == MV_OK);
    assert(mv_process_ref_resolve(later, child_ref, &result) == MV_INVALID_ARGUMENT && result == 999);
    mv_process_handle_destroy(shutdown_handle);
    mv_process_handle_destroy(returned_handle);
    mv_process_handle_destroy(failed_handle);
    mv_process_handle_destroy(owner_handle);
    mv_process_handle_destroy(handle);
    mv_process_handle_destroy(NULL);
    mv_context_destroy(later);
    mv_context_destroy(other);
    return 0;
}
