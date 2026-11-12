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

#include <math.h>
#include <stdlib.h>
#include "../core/process.h"
#include "malvado/platform.h"

int
mv_image_region_valid(const MvImage *image, MvImageRegion region, MvImageTransform transform)
{
    int width = mv_image_width(image);
    int height = mv_image_height(image);
    if (!image || region.x < 0 || region.y < 0 || region.width <= 0 || region.height <= 0 ||
        region.x >= width || region.y >= height ||
        region.width > width - region.x || region.height > height - region.y)
        return 0;
    if (!isfinite(transform.x) || !isfinite(transform.y) ||
        !isfinite(transform.pivot_x) || !isfinite(transform.pivot_y) ||
        !isfinite(transform.scale) || !isfinite(transform.rotation) || transform.scale <= 0)
        return 0;
    return isfinite(region.width * transform.scale) &&
           isfinite(region.height * transform.scale) &&
           isfinite(transform.pivot_x * transform.scale) &&
           isfinite(transform.pivot_y * transform.scale);
}

typedef struct {
    MvImageGraphic graphic;
    void *owner;
    MvResourceDestroy release;
    size_t count;
    MvCollisionShape shape;
    MvImageGraphic frames[];
} OwnedImageGraph;

static void
release_image_graph(void *data)
{
    OwnedImageGraph *graph = data;
    if (graph->count) {
        for (size_t i = 0; i < graph->count; ++i) mv_image_destroy(graph->frames[i].image);
    } else mv_image_destroy(graph->graphic.image);
    /* Release the lifetime owner after the final native texture use. */
    if (graph->release) graph->release(graph->owner);
    free(graph);
}

MvResult
mv_process_image_set_owned(MvContext *ctx, MvProcessRef process, MvImageGraphic graphic,
                           void *owner, MvResourceDestroy release)
{
    if ((owner != NULL) != (release != NULL)) return MV_INVALID_ARGUMENT;
    void *previous;
    MvResult result = malvado_process_visual_get(ctx, process, &previous);
    if (result != MV_OK) return result;
    MvCollisionShape shape;
    MvImageTransform transform = {0, 0, (float)graphic.pivot_x, (float)graphic.pivot_y, 1, 0};
    if (!mv_image_region_valid(graphic.image, graphic.region, transform) ||
        !mv_graphic_sprite(graphic.region.width, graphic.region.height,
                           graphic.pivot_x, graphic.pivot_y, &shape))
        return MV_INVALID_ARGUMENT;
    OwnedImageGraph *owned = malloc(sizeof(*owned));
    if (!owned) return MV_OUT_OF_MEMORY;
    if (!mv_image_retain(graphic.image)) { free(owned); return MV_OVERFLOW; }
    *owned = (OwnedImageGraph){.graphic = graphic, .owner = owner, .release = release};
    result = malvado_process_visual_set(ctx, process, shape, owned, release_image_graph);
    if (result != MV_OK) {
        /* A failed assignment does not take the lifetime owner. */
        mv_image_destroy(graphic.image);
        free(owned);
    }
    return result;
}

MvResult
mv_process_image_set(MvContext *ctx, MvProcessRef process, MvImageGraphic graphic)
{
    return mv_process_image_set_owned(ctx, process, graphic, NULL, NULL);
}

MvResult
mv_process_image_get(MvContext *ctx, MvProcessRef process, MvImageGraphic *out, int *present)
{
    if (!out || !present) return MV_INVALID_ARGUMENT;
    void *data;
    MvResult result = malvado_process_visual_get(ctx, process, &data);
    if (result != MV_OK) return result;
    if (data) *out = ((OwnedImageGraph *)data)->graphic;
    *present = data != NULL;
    return MV_OK;
}

static int
select_image_frame(void *data, size_t index, MvCollisionShape *shape)
{
    OwnedImageGraph *graph = data;
    if (index >= graph->count) return 0;
    MvImageGraphic frame = graph->frames[index];
    *shape = graph->shape;
    graph->graphic = frame;
    return 1;
}

MvResult
mv_process_image_animation_set_owned(MvContext *ctx, MvProcessRef process,
    const MvImageAnimationFrame *frames, size_t count, int loop, unsigned tick_rate,
    void *owner, MvResourceDestroy release)
{
    if ((owner != NULL) != (release != NULL) || !frames || !count || count > 8192 ||
        (loop != 0 && loop != 1) || !tick_rate || tick_rate > 1000000)
        return MV_INVALID_ARGUMENT;
    void *previous;
    MvResult result = malvado_process_visual_get(ctx, process, &previous);
    if (result != MV_OK) return result;
    double left = 0, top = 0, right = 0, bottom = 0;
    for (size_t i = 0; i < count; ++i) {
        MvImageGraphic frame = frames[i].graphic;
        MvCollisionShape shape;
        MvImageTransform transform = {0,0,(float)frame.pivot_x,(float)frame.pivot_y,1,0};
        if (!frames[i].duration_us || frames[i].duration_us > 3600000000ULL ||
            !mv_image_region_valid(frame.image, frame.region, transform) ||
            !mv_graphic_sprite(frame.region.width, frame.region.height, frame.pivot_x, frame.pivot_y, &shape))
            return MV_INVALID_ARGUMENT;
        double x = -frame.pivot_x, y = -frame.pivot_y;
        double end_x = frame.region.width - frame.pivot_x;
        double end_y = frame.region.height - frame.pivot_y;
        if (!i || x < left) left = x;
        if (!i || y < top) top = y;
        if (!i || end_x > right) right = end_x;
        if (!i || end_y > bottom) bottom = end_y;
    }
    MvAnimationFrame *timing = malloc(count * sizeof(*timing));
    if (!timing) return MV_OUT_OF_MEMORY;
    for (size_t i = 0; i < count; ++i)
        timing[i] = (MvAnimationFrame){(int)i + 1, frames[i].duration_us};
    MvAnimation *animation = mv_animation_create(timing, count, loop);
    free(timing);
    if (!animation) return MV_OUT_OF_MEMORY;
    OwnedImageGraph *owned = calloc(1, sizeof(*owned) + count * sizeof(*owned->frames));
    if (!owned) {
        mv_animation_destroy(animation);
        return MV_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; ++i) {
        if (!mv_image_retain(frames[i].graphic.image)) {
            for (size_t j = 0; j < i; ++j) mv_image_destroy(owned->frames[j].image);
            free(owned);
            mv_animation_destroy(animation);
            return MV_OVERFLOW;
        }
        owned->frames[i] = frames[i].graphic;
    }
    owned->shape = (MvCollisionShape){.kind = MV_COLLISION_RECT,
        .width = right - left, .height = bottom - top,
        .offset_x = (right + left) / 2, .offset_y = (bottom + top) / 2};
    owned->count = count;
    owned->owner = owner;
    owned->release = release;
    result = malvado_process_visual_animation_set(ctx, process, animation, tick_rate,
        owned, release_image_graph, select_image_frame);
    mv_animation_destroy(animation);
    if (result != MV_OK) {
        owned->release = NULL; /* Failure leaves the caller's owner intact. */
        release_image_graph(owned);
    }
    return result;
}

MvResult
mv_process_image_animation_set(MvContext *ctx, MvProcessRef process,
    const MvImageAnimationFrame *frames, size_t count, int loop, unsigned tick_rate)
{
    return mv_process_image_animation_set_owned(ctx, process, frames, count, loop, tick_rate, NULL, NULL);
}
