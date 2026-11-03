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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int
main(int argc, char **argv)
{
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    MvRuntimeId id, again = {1, 2, 3};
    assert(mv_context_runtime_id(NULL, &again) == MV_INVALID_ARGUMENT);
    assert(again.high == 1 && again.low == 2 && again.sequence == 3);
    assert(mv_context_runtime_id(ctx, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_context_runtime_id(ctx, &id) == MV_OK);
    assert(mv_context_runtime_id(ctx, &again) == MV_OK);
    assert(id.high == again.high && id.low == again.low && id.sequence == again.sequence);
    if (argc == 4) {
        MvProcessId process, resolved = 99;
        assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &process) == MV_OK && process == 1);
        MvProcessRef foreign = {{strtoull(argv[1], NULL, 16), strtoull(argv[2], NULL, 16),
                                 strtoull(argv[3], NULL, 16)}, 1};
        assert(mv_process_ref_resolve(ctx, foreign, &resolved) == MV_INVALID_ARGUMENT);
        assert(resolved == 99);
    } else assert(argc == 1);
    printf("%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "\n", id.high, id.low, id.sequence);
    mv_context_destroy(ctx);
    return 0;
}
