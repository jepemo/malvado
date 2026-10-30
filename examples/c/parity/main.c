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
#include <string.h>

typedef struct {
    MvProcessId worker;
    unsigned tick;
    int worker_closed;
} Game;

static MvStepResult
worker_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)user;
    MvMessage message;
    if (mv_process_receive(ctx, id, &message) != MV_OK)
        return MV_STEP_ERROR;
    int valid = message.type == MV_MESSAGE_BYTES && message.value.bytes.size == 4 &&
                memcmp(message.value.bytes.data, "ping", 4) == 0;
    mv_message_release(&message);
    if (!valid)
        return MV_STEP_ERROR;
    puts("worker: ping");
    return MV_STEP_YIELD;
}

static void
worker_destroy(void *user)
{
    Game *game = user;
    game->worker_closed++;
    puts("worker: closed");
}

static MvStepResult
controller_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    Game *game = user;
    game->tick++;
    if (game->tick == 1) {
        MvMessage message = {.type = MV_MESSAGE_BYTES, .value.bytes = {(void *)"ping", 4}};
        if (mv_process_send(ctx, game->worker, &message) != MV_OK ||
            mv_process_signal(ctx, game->worker, MV_SIGNAL_FREEZE, 0) != MV_OK)
            return MV_STEP_ERROR;
        puts("controller: freeze");
    } else if (game->tick == 2) {
        if (mv_process_signal(ctx, game->worker, MV_SIGNAL_WAKEUP, 0) != MV_OK)
            return MV_STEP_ERROR;
        puts("controller: wake");
    } else {
        if (mv_process_signal(ctx, game->worker, MV_SIGNAL_KILL, 0) != MV_OK)
            return MV_STEP_ERROR;
        puts("controller: kill");
        return MV_STEP_DONE;
    }
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    Game game = {0};
    MvProcessId controller;
    int ok = mv_process_spawn(ctx, 0, 0, worker_step, worker_destroy, &game, &game.worker) == MV_OK &&
             mv_process_spawn(ctx, 0, 10, controller_step, NULL, &game, &controller) == MV_OK;
    while (ok && mv_process_count(ctx))
        ok = mv_process_tick(ctx) == MV_OK;
    mv_context_destroy(ctx);
    return ok && game.worker_closed == 1 ? 0 : 1;
}
