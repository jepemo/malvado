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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include "malvado/platform.h"
#include "raylib.h"
#include <assert.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static void pixel(int x, int y, int r, int g)
{
    mv_window_flush();
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color color = GetImageColor(image, x * image.width / GetScreenWidth(), y * image.height / GetScreenHeight());
    UnloadImage(image);
    assert(color.r == r && color.g == g);
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4);
    assert(ctx);
    MvProcessRef refs[2];
    MvGraphic graphic = {.color = {255, 0, 0, 255}};
    assert(mv_graphic_rect(10, 4, 0, &graphic.shape));
    for (unsigned i = 0; i < 2; ++i) {
        MvProcessId id;
        assert(mv_process_spawn(ctx, 0, 100 - i, step, NULL, NULL, &id) == MV_OK);
        assert(mv_process_ref_create(ctx, id, &refs[i]) == MV_OK);
        assert(mv_process_set_position(ctx, refs[i], 20, 20) == MV_OK);
        graphic.color = i ? (MvColor){0, 255, 0, 255} : (MvColor){255, 0, 0, 255};
        assert(mv_process_graphic_set(ctx, refs[i], graphic) == MV_OK);
    }
    assert(!mv_window_draw_process_graphics(ctx));
    assert(mv_window_open(64, 64, "Native process graphics"));
    MvColor black = {0, 0, 0, 255};
    mv_window_begin(black);
    assert(!mv_window_draw_process_graphics(NULL));
    assert(mv_window_draw_process_graphics(ctx));
    pixel(20, 20, 0, 255);
    mv_window_end();
    assert(mv_process_set_z(ctx, refs[0], 1) == MV_OK);
    assert(mv_process_set_angle(ctx, refs[0], 90) == MV_OK);
    assert(mv_process_set_scale(ctx, refs[0], 2) == MV_OK);
    assert(mv_process_signal(ctx, refs[0].id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_signal(ctx, refs[1].id, MV_SIGNAL_SLEEP, 0) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(20, 27, 255, 0);
    pixel(27, 20, 0, 0);
    mv_window_end();
    assert(mv_process_set_scale(ctx, refs[0], 0) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(20, 20, 0, 0);
    mv_window_end();
    assert(mv_process_set_scale(ctx, refs[0], 1) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_clip_begin(0, 0, 10, 10));
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_window_clip_end());
    pixel(20, 20, 0, 0);
    assert(mv_window_scroll_begin(-20, -20));
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_window_scroll_end());
    pixel(40, 40, 255, 0);
    mv_window_end();
    assert(mv_process_graphic_clear(ctx, refs[0]) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(20, 20, 0, 0);
    mv_window_end();
    assert(!mv_window_draw_process_graphics(ctx));
    mv_context_destroy(ctx);
    mv_window_close();
    return 0;
}
