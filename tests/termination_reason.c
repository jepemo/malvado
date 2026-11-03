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
#include "core/process.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

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
    assert(ctx);
    MvStepResult done = MV_STEP_DONE, wait = MV_STEP_YIELD, error = MV_STEP_ERROR;
    MvProcessId parent, child, failed;
    assert(mv_process_spawn(ctx, 0, 1, step, NULL, &done, &parent) == MV_OK);
    assert(mv_process_spawn(ctx, parent, 0, step, NULL, &wait, &child) == MV_OK);
    MvEndReason reason = MV_END_ERROR;
    assert(!mv_process_end_reason(ctx, parent, &reason) && reason == MV_END_ERROR);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_end_reason(ctx, parent, &reason) && reason == MV_END_RETURNED);
    assert(mv_process_end_reason(ctx, child, &reason) && reason == MV_END_KILLED);
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, &error, &failed) == MV_OK);
    assert(mv_process_tick(ctx) == MV_PROCESS_ERROR);
    assert(mv_process_end_reason(ctx, failed, &reason) && reason == MV_END_ERROR);
    assert(!mv_process_end_reason(ctx, parent, &reason));
    mv_context_destroy(ctx);
    return 0;
}
