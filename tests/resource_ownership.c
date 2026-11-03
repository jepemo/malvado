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
    assert(ctx);
    MvProcessId owner;
    assert(mv_process_spawn(ctx, 0, 0, finish, NULL, NULL, &owner) == MV_OK);
    MvProcessRef owner_ref;
    assert(mv_process_ref_create(ctx, owner, &owner_ref) == MV_OK);
    Data owned = {ctx, 0}, shared = {NULL, 0}, game = {NULL, 0};
    MvResource *a, *b, *c;
    assert(mv_resource_create(ctx, &owner_ref, &owned, release_data, &a) == MV_OK);
    assert(mv_resource_create(ctx, &owner_ref, &shared, release_data, &b) == MV_OK);
    assert(mv_resource_create(ctx, NULL, &game, release_data, &c) == MV_OK);
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
    assert(mv_resource_transfer(c, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_resource_release(c) == MV_OK && game.destroyed == 1);

    MvContext *first = mv_context_create(60, 5);
    MvContext *foreign = mv_context_create(60, 5);
    assert(first && foreign);
    MvProcessId old_owner, new_owner, foreign_owner;
    assert(mv_process_spawn(first, 0, 0, wait_step, NULL, NULL, &old_owner) == MV_OK);
    assert(mv_process_spawn(first, 0, 0, wait_step, NULL, NULL, &new_owner) == MV_OK);
    assert(mv_process_spawn(foreign, 0, 0, wait_step, NULL, NULL, &foreign_owner) == MV_OK);
    MvProcessRef old_ref, new_ref, foreign_ref;
    assert(mv_process_ref_create(first, old_owner, &old_ref) == MV_OK);
    assert(mv_process_ref_create(first, new_owner, &new_ref) == MV_OK);
    assert(mv_process_ref_create(foreign, foreign_owner, &foreign_ref) == MV_OK);
    Data moved = {NULL, 0}, owned_by_game = {NULL, 0}, promoted = {NULL, 0};
    MvResource *moving, *game_resource, *promoted_resource;
    assert(mv_resource_create(first, &old_ref, &moved, release_data, &moving) == MV_OK);
    assert(mv_resource_create(first, NULL, &owned_by_game, release_data, &game_resource) == MV_OK);
    assert(mv_resource_create(first, &new_ref, &promoted, release_data, &promoted_resource) == MV_OK);
    assert(mv_resource_transfer(promoted_resource, NULL) == MV_OK);
    MvResource *unchanged = moving;
    assert(mv_resource_create(first, &foreign_ref, &moved, release_data, &unchanged) == MV_INVALID_ARGUMENT);
    assert(unchanged == moving);
    assert(mv_resource_retain(moving) == MV_OK);
    assert(mv_resource_transfer(moving, &foreign_ref) == MV_INVALID_ARGUMENT);
    assert(mv_resource_transfer(game_resource, &old_ref) == MV_OK);
    assert(mv_process_signal(first, old_owner, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_tick(first) == MV_OK);
    assert(moved.destroyed == 0 && owned_by_game.destroyed == 1);
    assert(mv_resource_create(first, &old_ref, &moved, release_data, &unchanged) == MV_NOT_FOUND);
    assert(unchanged == moving);
    assert(mv_resource_transfer(moving, &old_ref) == MV_NOT_FOUND);
    assert(mv_resource_transfer(moving, &new_ref) == MV_OK);
    assert(mv_resource_release(moving) == MV_OK && moved.destroyed == 0);
    assert(mv_process_signal(first, new_owner, MV_SIGNAL_KILL, 0) == MV_OK);
    assert(mv_process_tick(first) == MV_OK && moved.destroyed == 1 && promoted.destroyed == 0);
    mv_context_destroy(first);
    assert(promoted.destroyed == 1);
    mv_context_destroy(foreign);
    return 0;
}
