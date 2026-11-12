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
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "malvado/platform.h"
#include "malvado/camera.h"
#include "raylib.h"

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

static void owner_released(void *data)
{
    ++*(int *)data;
}

int
main(void)
{
    const uint8_t pixels[] = {255, 0, 0, 255, 0, 0, 0, 0,
                              0, 255, 0, 255, 0, 0, 255, 255};
    assert(!mv_image_from_rgba(2, 2, pixels, sizeof(pixels)));
    assert(mv_window_open(64, 64, "RGBA image"));
    assert(!mv_image_from_rgba(0, 2, pixels, sizeof(pixels)));
    assert(!mv_image_from_rgba(INT_MAX, 2, pixels, sizeof(pixels)));
    assert(!mv_image_from_rgba(2, 2, NULL, sizeof(pixels)));
    assert(!mv_image_from_rgba(2, 2, pixels, sizeof(pixels) - 1));
    Image source = {.data = (void *)pixels, .width = 2, .height = 2,
                    .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    int png_size = 0;
    unsigned char *png = ExportImageToMemory(source, ".png", &png_size);
    assert(png && png_size > 24);
    assert(!mv_image_from_png(NULL, (size_t)png_size));
    assert(!mv_image_from_png(png, SIZE_MAX));
    for (int size = 0; size < 24; ++size)
        assert(!mv_image_from_png(png, (size_t)size));
    unsigned char saved = png[0];
    png[0] = 0;
    assert(!mv_image_from_png(png, (size_t)png_size));
    png[0] = saved;
    saved = png[16];
    png[16] = 1;
    assert(!mv_image_from_png(png, (size_t)png_size));
    png[16] = saved;
    MvImage *decoded = mv_image_from_png(png, (size_t)png_size);
    assert(decoded && mv_image_width(decoded) == 2 && mv_image_height(decoded) == 2);
    MemFree(png);
    assert(!mv_image_retain(NULL));
    assert(mv_image_retain(decoded));
    mv_image_destroy(decoded);
    assert(mv_image_width(decoded) == 2);

    MvImage *image = mv_image_from_rgba(2, 2, pixels, sizeof(pixels));
    assert(image && mv_image_width(image) == 2 && mv_image_height(image) == 2);
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    MvProcessId id;
    MvProcessRef ref;
    assert(ctx && other);
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    MvImageGraphic graph = {decoded, {0, 0, 2, 2}, 0.5, 0}, out = {0};
    int present = 42;
    assert(mv_process_image_get(ctx, ref, &out, &present) == MV_OK && !present);
    assert(mv_process_image_set(other, ref, graph) == MV_INVALID_ARGUMENT);
    int released = 0;
    assert(mv_process_image_set_owned(other, ref, graph, &released, owner_released) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_set_owned(ctx, ref, graph, &released, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_set_owned(ctx, ref, graph, NULL, owner_released) == MV_INVALID_ARGUMENT);
    assert(released == 0);
    assert(mv_process_image_set_owned(ctx, ref, graph, &released, owner_released) == MV_OK);
    mv_image_destroy(decoded); /* Only the process now retains this texture. */
    assert(mv_process_image_get(ctx, ref, &out, &present) == MV_OK && present);
    assert(out.image == graph.image && out.pivot_x == 0.5 && out.region.width == 2);
    MvImageGraphic invalid = graph;
    invalid.pivot_x = NAN;
    assert(mv_process_image_set(ctx, ref, invalid) == MV_INVALID_ARGUMENT);
    invalid = graph;
    invalid.region.width = 3;
    assert(mv_process_image_set(ctx, ref, invalid) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_get(ctx, ref, NULL, &present) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_get(ctx, ref, &out, &present) == MV_OK && present && out.region.width == 2);
    MvGraphic primitive;
    assert(mv_process_graphic_get(ctx, ref, &primitive, &present) == MV_OK && !present);
    assert(mv_process_set_position(ctx, ref, 40, 10) == MV_OK);
    assert(mv_process_set_scale(ctx, ref, 10) == MV_OK);
    MvCameraViewport region = {30, 10, 10, 20};
    assert(mv_process_region_set(ctx, ref, &region) == MV_OK);
    assert(mv_process_scroll_set(ctx, ref, 2, 0) == MV_OK);
    MvProcessId target_id;
    MvProcessRef target;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &target_id) == MV_OK);
    assert(mv_process_ref_create(ctx, target_id, &target) == MV_OK);
    MvGraphic target_graph = {.color = {0, 0, 255, 255}};
    assert(mv_graphic_circle(1, &target_graph.shape));
    assert(mv_process_graphic_set(ctx, target, target_graph) == MV_OK);
    assert(mv_process_set_position(ctx, target, 35, 15) == MV_OK);
    int overlap;
    assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && overlap);
    assert(mv_process_set_position(ctx, ref, 60, 10) == MV_OK);
    assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && !overlap);
    assert(mv_process_set_position(ctx, ref, 40, 10) == MV_OK);
    assert(mv_process_set_z(ctx, ref, 1) == MV_OK);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_SLEEP, 0) == MV_OK);
    mv_window_begin((MvColor){30, 40, 50, 255});
    assert(mv_window_draw_process_graphics(ctx));
    Image sleeping = LoadImageFromScreen();
    Color hidden = GetImageColor(sleeping, 32 * sleeping.width / 64, 15 * sleeping.height / 64);
    assert(hidden.r == 30 && hidden.g == 40 && hidden.b == 50);
    UnloadImage(sleeping);
    mv_window_end();
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    mv_window_begin((MvColor){30, 40, 50, 255});
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    assert(released == 1);
    assert(mv_image_draw_region(image, (MvImageRegion){0, 0, 2, 2},
                                (MvImageTransform){10, 10, 0, 0, 10, 0}));
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int w = screen.width, h = screen.height;
    Color red = GetImageColor(screen, 15 * w / 64, 15 * h / 64);
    Color clear = GetImageColor(screen, 25 * w / 64, 15 * h / 64);
    Color green = GetImageColor(screen, 15 * w / 64, 25 * h / 64);
    Color process_red = GetImageColor(screen, 35 * w / 64, 15 * h / 64);
    assert(process_red.r > 240 && process_red.g < 10 && process_red.b < 10);
    Color clipped = GetImageColor(screen, 45 * w / 64, 15 * h / 64);
    assert(clipped.r == 30 && clipped.g == 40 && clipped.b == 50);
    Color blue = GetImageColor(screen, 25 * w / 64, 25 * h / 64);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(clear.r == 30 && clear.g == 40 && clear.b == 50);
    assert(green.g > 240 && green.r < 10 && green.b < 10);
    assert(blue.b > 240 && blue.r < 10 && blue.g < 10);
    UnloadImage(screen);
    mv_window_end();
    assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    assert(mv_process_image_get(ctx, ref, &out, &present) == MV_OK && !present);
    graph.image = image;
    assert(mv_process_image_set_owned(ctx, ref, graph, &released, owner_released) == MV_OK);
    mv_image_destroy(image);
    int killed;
    assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_image_get(ctx, ref, &out, &present) == MV_NOT_FOUND);
    assert(released == 2);
    image = mv_image_from_rgba(2, 2, pixels, sizeof(pixels));
    assert(image);
    graph.image = image;
    assert(mv_process_image_set_owned(ctx, target, graph, &released, owner_released) == MV_OK);
    assert(mv_process_graphic_set(ctx, target, target_graph) == MV_OK && released == 3);
    assert(mv_process_image_set_owned(ctx, target, graph, &released, owner_released) == MV_OK);
    mv_image_destroy(image);
    mv_context_destroy(ctx);
    assert(released == 4);
    mv_context_destroy(other);
    mv_window_close();
    return 0;
}
