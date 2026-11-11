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
} OwnedImageGraph;

static void
release_image_graph(void *data)
{
    OwnedImageGraph *graph = data;
    mv_image_destroy(graph->graphic.image);
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
    *owned = (OwnedImageGraph){graphic, owner, release};
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
