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
#include <stdio.h>
#include <time.h>

enum { DEPTH = 512, QUERIES = 10 };

static MvStepResult
wait_step(MvContext *ctx, MvProcessId id, void *user)
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
    if (!ctx) return 1;
    MvProcessId root, owner, child;
    if (mv_process_spawn(ctx, 0, 0, wait_step, NULL, NULL, &root) != MV_OK) return 1;
    owner = root;
    for (int i = 0; i < DEPTH; ++i) {
        if (mv_process_spawn(ctx, owner, 0, wait_step, NULL, NULL, &child) != MV_OK) return 1;
        owner = child;
    }
    clock_t start = clock();
    for (int i = 0; i < QUERIES; ++i) {
        size_t total;
        if (mv_process_descendants(ctx, root, 0, NULL, 0, &total) != MV_OK || total != DEPTH)
            return 1;
    }
    clock_t end = clock();
    if (start == (clock_t)-1 || end == (clock_t)-1) return 1;
    printf("depth=%d queries=%d cpu_ms=%.3f\n", DEPTH, QUERIES,
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    mv_context_destroy(ctx);
    return 0;
}
