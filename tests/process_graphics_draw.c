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
#include "malvado/camera.h"
#include "../src/platform/render_internal.h"
#include <float.h>
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
static void released(void *data) { ++*(int *)data; }

static void command_checks(MvContext *ctx, MvProcessRef ref)
{
    MvColor red = {255,0,0,255}, green = {0,255,0,255}, black = {0,0,0,255};
    assert(mv_process_set_scale(ctx,ref,0) == MV_OK);
    assert(mv_process_set_position(ctx,ref,1e30,1e30) == MV_OK);
    assert(mv_process_draw_box(ctx,ref,10,10,20,20,1,red) == MV_OK);
    const uint8_t rgba[] = {0,255,0,255};
    MvImage *image = mv_image_from_rgba(1,1,rgba,sizeof(rgba));
    assert(image);
    MvImageRegion region = {0,0,1,1};
    MvImageTransform transform = {12,12,0,0,4,0};
    int releases = 0;
    assert(mv_process_draw_image_region_owned(ctx,ref,image,region,transform,&releases,NULL) == MV_INVALID_ARGUMENT);
    MvImageRegion bad = {0,0,2,1};
    assert(mv_process_draw_image_region_owned(ctx,ref,image,bad,transform,&releases,released) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_image_region_owned(ctx,ref,image,region,transform,&releases,released) == MV_OK);
    mv_image_destroy(image);
    assert(mv_process_draw_circle(ctx,ref,24,24,2,1,green) == MV_OK);
    char text[] = "Native";
    assert(mv_process_draw_text(ctx,ref,text,2,40,-1,green) == MV_OK);
    text[0] = 'X';
    size_t count;
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 4);
    assert(!mv_window_draw_process_commands(ctx,ref));
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(11,11,255,0);
    pixel(13,13,0,255);
    pixel(24,24,0,255);
    mv_window_draw_text("Native",2,50,10,green);
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    int lit = 0;
    for (int y = 0; y < 10; ++y) for (int x = 2; x < 38; ++x) {
        Color a = GetImageColor(screen,x*screen.width/GetScreenWidth(),(40+y)*screen.height/GetScreenHeight());
        Color b = GetImageColor(screen,x*screen.width/GetScreenWidth(),(50+y)*screen.height/GetScreenHeight());
        assert(a.r == b.r && a.g == b.g && a.b == b.b);
        if (a.g) lit = 1;
    }
    assert(lit);
    UnloadImage(screen);
    mv_window_end();
    assert(mv_process_tick(ctx) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_clip_begin(0,0,28,28));
    assert(mv_window_scroll_begin(-10,0));
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_window_scroll_end());
    assert(mv_window_clip_end());
    pixel(23,13,0,255);
    pixel(29,13,0,0);
    mv_window_end();
    assert(mv_process_signal(ctx,ref.id,MV_SIGNAL_SLEEP,0) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(13,13,0,0);
    mv_window_end();
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 0);
}

typedef struct {
    MvContext *ctx;
    MvProcessRef *refs;
    MvImage *queued;
    unsigned calls;
} Preparation;
static int prepare_graph(MvProcessRef ref, void *user)
{
    Preparation *state = user;
    assert(ref.id == state->refs[state->calls].id);
    ++state->calls;
    if (state->queued) {
        mv_image_destroy(state->queued);
        state->queued = NULL;
    }
    if (ref.id == state->refs[0].id) return 0;
    assert(mv_process_set_position(state->ctx,ref,40,40) == MV_OK);
    return 1;
}
static void preparation_checks(MvContext *ctx, MvProcessRef *refs)
{
    uint8_t rgba[] = {255,0,0,255};
    MvImage *queued = mv_image_from_rgba(1,1,rgba,sizeof(rgba));
    assert(queued);
    Preparation state = {ctx,refs,queued,0};
    assert(mv_process_draw_box(ctx,refs[0],4,4,2,2,1,(MvColor){255,0,0,255}) == MV_OK);
    assert(!malvado_window_draw_process_graphics(ctx,prepare_graph,&state) && !state.calls);
    mv_window_begin((MvColor){0,0,0,255});
    mv_image_draw(queued,50,50);
    assert(malvado_window_draw_process_graphics(ctx,prepare_graph,&state));
    assert(state.calls == 2 && !state.queued);
    pixel(4,4,255,0); /* Graph suppression must not hide stored commands. */
    pixel(20,20,0,0);
    pixel(40,40,0,255); /* Native transform is read after preparation. */
    pixel(50,50,255,0); /* Queued texture was flushed before callback release. */
    mv_window_end();
    assert(mv_process_draw_clear(ctx,refs[0]) == MV_OK);
    assert(mv_process_set_position(ctx,refs[1],20,20) == MV_OK);
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
    preparation_checks(ctx,refs);
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
    MvCameraRef main;
    MvCamera camera;
    assert(mv_camera_main(ctx, &main) == MV_OK);
    assert(mv_camera_init(&camera, FLT_MAX / 64, 0, NULL));
    assert(mv_camera_put(ctx, main, camera) == MV_OK);
    assert(mv_process_set_position(ctx, refs[0], (double)(FLT_MAX / 16) + FLT_MAX / 64, 20) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_scroll_begin(FLT_MAX / 16, 0));
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_window_scroll_end());
    pixel(0, 20, 0, 0);
    mv_window_end();
    assert(mv_camera_init(&camera, 0, 0, NULL));
    assert(mv_camera_put(ctx, main, camera) == MV_OK);
    assert(mv_process_set_position(ctx, refs[0], 20, 20) == MV_OK);
    assert(mv_process_graphic_clear(ctx, refs[0]) == MV_OK);
    mv_window_begin(black);
    assert(mv_window_draw_process_graphics(ctx));
    pixel(20, 20, 0, 0);
    mv_window_end();
    assert(!mv_window_draw_process_graphics(ctx));
    command_checks(ctx, refs[0]);
    mv_context_destroy(ctx);
    mv_window_close();
    return 0;
}
