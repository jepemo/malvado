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
#include "malvado/platform.h"

typedef struct { int ticks; } Game;

static MvStepResult
step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    Game *game = user;
    ++game->ticks;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx) return 1;
    if (!mv_window_open(640, 360, "Malvado C")) {
        mv_context_destroy(ctx);
        return 1;
    }
    Game game = {0};
    MvProcessId id;
    int ok = mv_process_spawn(ctx, 0, 0, step, NULL, &game, &id) == MV_OK;
    while (ok && !mv_window_should_close()) {
        ok = mv_process_tick(ctx) == MV_OK;
        mv_window_begin((MvColor){25, 27, 40, 255});
        mv_window_draw_text("Hello, Malvado C", 40, 40, 30, (MvColor){255, 255, 255, 255});
        mv_window_end();
    }
    mv_window_close();
    mv_context_destroy(ctx);
    return ok && game.ticks ? 0 : 1;
}
