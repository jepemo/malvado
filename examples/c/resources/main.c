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

typedef struct {
    int value;
    int destroyed;
} Asset;

static void
release_asset(void *data)
{
    Asset *asset = data;
    asset->destroyed++;
}

static MvStepResult
finish(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_DONE;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    Asset asset = {42, 0};
    MvProcessId id;
    MvProcessRef owner;
    MvResource *resource;
    if (mv_process_spawn(ctx, 0, 0, finish, NULL, NULL, &id) != MV_OK ||
        mv_process_ref_create(ctx, id, &owner) != MV_OK ||
        mv_resource_create(ctx, &owner, &asset, release_asset, &resource) != MV_OK) {
        mv_context_destroy(ctx);
        return 1;
    }
    if (mv_resource_retain(resource) != MV_OK) {
        mv_context_destroy(ctx);
        return 1;
    }
    int ok = mv_process_tick(ctx) == MV_OK && asset.destroyed == 0 &&
             mv_resource_data(resource) == &asset;
    if (ok)
        printf("Retained asset: %d\n", asset.value);
    if (mv_resource_release(resource) != MV_OK)
        ok = 0;
    ok = ok && asset.destroyed == 1;
    mv_context_destroy(ctx);
    return ok ? 0 : 1;
}
