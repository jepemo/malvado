/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#include <math.h>
#include <stdio.h>
#include "malvado/malvado.h"
#include "malvado/platform.h"
#include "malvado/graphic.h"

#define WIDTH 640
#define HEIGHT 360
#define TICKS_PER_SECOND 60.0

/* Each process owns its reference. The ball also keeps its target and score. */
typedef struct {
    MvProcessRef self;
    MvProcessRef target;
    int score;
} Actor;

/* Move the player left and right with the arrow keys and clamp its center. */
static MvStepResult
player_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    Actor *player = user;
    double x, y;
    if (mv_process_position(ctx, player->self, &x, &y) != MV_OK)
        return MV_STEP_ERROR;

    double distance = 400.0 / TICKS_PER_SECOND;
    if (mv_key_down(MV_KEY_LEFT))
        x -= distance;
    if (mv_key_down(MV_KEY_RIGHT))
        x += distance;
    if (x < 40.0)
        x = 40.0;
    if (x > WIDTH - 40.0)
        x = WIDTH - 40.0;
    if (mv_process_set_position(ctx, player->self, x, y) != MV_OK)
        return MV_STEP_ERROR;

    return mv_window_escape_down() ? MV_STEP_DONE : MV_STEP_YIELD;
}

/* Fall, score one point on a catch, and respawn above the screen. */
static MvStepResult
ball_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)id;
    Actor *ball = user;
    double x, y;
    if (mv_process_position(ctx, ball->self, &x, &y) != MV_OK)
        return MV_STEP_ERROR;

    y += 180.0 / TICKS_PER_SECOND;

    int caught = 0;
    if (mv_process_collider_test(ctx, ball->self, ball->target, &caught) != MV_OK)
        return MV_STEP_ERROR;
    if (caught)
        ball->score++;
    if (caught || y > HEIGHT + 12.0) {
        x = fmod(x + 160.0, (double)WIDTH);
        y = -12.0;
    }
    if (mv_process_set_position(ctx, ball->self, x, y) != MV_OK)
        return MV_STEP_ERROR;

    /* The engine clears each process command list before its next activation. */
    char text[32];
    snprintf(text, sizeof text, "Score: %d", ball->score);
    if (mv_process_draw_text(ctx, ball->self, text, 20, 20, 24,
                             (MvColor){255, 255, 255, 255}) != MV_OK)
        return MV_STEP_ERROR;

    return mv_window_escape_down() ? MV_STEP_DONE : MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(TICKS_PER_SECOND, 5);
    if (!ctx)
        return 1;
    if (!mv_window_open(WIDTH, HEIGHT, "Catch!")) {
        mv_context_destroy(ctx);
        return 1;
    }

    Actor player = {0};
    Actor ball = {0};
    MvProcessId player_id, ball_id;
    int ok = mv_process_spawn(ctx, 0, 0, player_step, NULL, &player, &player_id) == MV_OK &&
             mv_process_spawn(ctx, 0, 0, ball_step, NULL, &ball, &ball_id) == MV_OK &&
             mv_process_ref_create(ctx, player_id, &player.self) == MV_OK &&
             mv_process_ref_create(ctx, ball_id, &ball.self) == MV_OK &&
             mv_process_ref_create(ctx, player_id, &ball.target) == MV_OK;

    MvGraphic rect = {.color = {0, 255, 255, 255}};
    MvGraphic circle = {.color = {255, 165, 0, 255}};
    ok = ok &&
         mv_graphic_rect(80, 16, 0, &rect.shape) &&
         mv_graphic_circle(12, &circle.shape) &&
         mv_process_set_position(ctx, player.self, WIDTH / 2.0, HEIGHT - 40.0) == MV_OK &&
         mv_process_set_position(ctx, ball.self, 80, 0) == MV_OK &&
         mv_process_graphic_set(ctx, player.self, rect) == MV_OK &&
         mv_process_graphic_set(ctx, ball.self, circle) == MV_OK;

    MvColor background = {0, 0, 0, 255};
    while (ok && mv_process_count(ctx) && !mv_window_should_close()) {
        ok = mv_process_tick(ctx) == MV_OK;
        mv_window_begin(background);
        ok = ok && mv_window_draw_process_graphics(ctx);
        mv_window_end();
    }

    mv_window_close();
    mv_context_destroy(ctx);
    return ok ? 0 : 1;
}
