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

typedef struct {
    MvContext *ctx;
    int destroyed;
} Data;

static void
release_data(void *value)
{
    Data *data = value;
    data->destroyed++;
    mv_context_destroy(data->ctx);
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
    assert(ctx);
    MvProcessId owner;
    assert(mv_process_spawn(ctx, 0, 0, finish, NULL, NULL, &owner) == MV_OK);
    Data owned = {ctx, 0}, shared = {NULL, 0}, game = {NULL, 0};
    MvResource *a, *b, *c;
    assert(mv_resource_create(ctx, owner, &owned, release_data, &a) == MV_OK);
    assert(mv_resource_create(ctx, owner, &shared, release_data, &b) == MV_OK);
    assert(mv_resource_create(ctx, 0, &game, release_data, &c) == MV_OK);
    assert(mv_resource_data(a) == &owned && mv_resource_data(b) == &shared);
    assert(mv_resource_release(a) == MV_INVALID_ARGUMENT);
    assert(mv_resource_retain(b) == MV_OK);
    assert(mv_resource_retain(b) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(owned.destroyed == 1 && shared.destroyed == 0 && game.destroyed == 0);
    assert(mv_resource_release(b) == MV_OK && shared.destroyed == 0);
    assert(mv_resource_release(b) == MV_OK && shared.destroyed == 1);
    assert(mv_resource_retain(c) == MV_OK);
    mv_context_destroy(ctx);
    assert(game.destroyed == 0);
    /* A detached reference must not use the already destroyed context. */
    game.ctx = NULL;
    assert(mv_resource_release(c) == MV_OK && game.destroyed == 1);
    return 0;
}
