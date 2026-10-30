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
#include <stdio.h>
#include <string.h>
#ifdef MALVADO_PARITY_CAPTURE
#include <stdlib.h>
#include "raylib.h"
#endif

typedef struct {
    unsigned ticks, elapsed, stopped_at;
    int smoke;
} Game;

static MvStepResult
game_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    Game *game = user;
    game->ticks++;
    if (!game->stopped_at) {
        game->elapsed++;
        if (mv_window_space_down() || (game->smoke && game->ticks == 5))
            game->stopped_at = game->elapsed;
    }
    return game->smoke && game->ticks >= 10 ? MV_STEP_DONE : MV_STEP_YIELD;
}

int
main(int argc, char **argv)
{
    Game game = {.smoke = argc == 2 && strcmp(argv[1], "--smoke") == 0};
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    if (!mv_window_open(640, 360, "Timing game")) {
        mv_context_destroy(ctx);
        return 1;
    }
    MvProcessId id;
    int ok = mv_process_spawn(ctx, 0, 0, game_step, NULL, &game, &id) == MV_OK;
#ifdef MALVADO_PARITY_CAPTURE
    int captured = 0;
    const char *capture_path = getenv("MALVADO_CAPTURE_PATH");
#endif
    if (ok)
        ok = mv_process_tick(ctx) == MV_OK;
    double last = mv_platform_time();
    while (ok && mv_process_count(ctx) && !mv_window_should_close() && !mv_window_escape_down()) {
        char text[96];
        snprintf(text, sizeof(text), "Time: %.2f seconds", game.elapsed / 60.0);
        mv_window_begin((MvColor){25, 30, 45, 255});
        mv_window_draw_text("Press SPACE as close to 5 seconds as you can.", 30, 40, 20,
                            (MvColor){255, 255, 255, 255});
        mv_window_draw_text(text, 30, 100, 28, (MvColor){255, 255, 255, 255});
        if (game.stopped_at) {
            double difference = (game.stopped_at > 300 ? game.stopped_at - 300 : 300 - game.stopped_at) / 60.0;
            snprintf(text, sizeof(text), "Difference: %.2f seconds. Press ESC to exit.", difference);
            mv_window_draw_text(text, 30, 160, 20, (MvColor){255, 255, 255, 255});
        }
#ifdef MALVADO_PARITY_CAPTURE
        if (capture_path && !captured && game.stopped_at == 5) {
            mv_window_flush();
            Image image = LoadImageFromScreen();
            captured = image.data && ExportImage(image, capture_path);
            UnloadImage(image);
            if (!captured)
                ok = 0;
        }
#endif
        mv_window_end();
        double now = mv_platform_time();
        unsigned steps = 0;
        ok = mv_context_advance(ctx, now - last, &steps) == MV_OK;
        last = now;
        for (unsigned i = 0; ok && i < steps && mv_process_count(ctx); ++i)
            ok = mv_process_tick(ctx) == MV_OK;
    }
    mv_window_close();
    mv_context_destroy(ctx);
#ifdef MALVADO_PARITY_CAPTURE
    if (capture_path && !captured)
        ok = 0;
#endif
    return ok && (!game.smoke || game.stopped_at) ? 0 : 1;
}
