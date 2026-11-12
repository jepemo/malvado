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

#ifndef MALVADO_GRAPHIC_H
#define MALVADO_GRAPHIC_H
#include "malvado/collision.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Copied colors and primitive graphics need no platform or resource owner. */
typedef struct { unsigned char r, g, b, a; } MvColor;
typedef struct { MvCollisionShape shape; MvColor color; } MvGraphic;
/* Assign a canonical rectangle/circle with copied color. No resource retention.
 * A new process has no primitive graphic. Get always writes present on success;
 * when absent, graphic is unchanged. Outputs must be non-NULL. Clear is
 * idempotent. Foreign/invalid inputs return MV_INVALID_ARGUMENT; dead processes
 * return MV_NOT_FOUND. Failure preserves state and outputs. Owner-thread only.
 * Fixed colliders remain independent; new processes derive colliders from this
 * primitive by default. A platform image graph replaces this primitive: get
 * reports absent while an image is selected. Set replaces either graph kind;
 * clear releases either kind. Automatic animation is not part of this service. */
MvResult mv_process_graphic_set(MvContext *ctx, MvProcessRef process, MvGraphic graphic);
MvResult mv_process_graphic_get(MvContext *ctx, MvProcessRef process, MvGraphic *graphic, int *present);
MvResult mv_process_graphic_clear(MvContext *ctx, MvProcessRef process);

/* Text alignment uses measured nonnegative pixel width. alignment is 0 (left),
 * 1 (center), or 2 (right). Center uses integer width/2. Saturate the result
 * to the int coordinate range. Return 1 on success; invalid width/alignment or
 * NULL output returns 0 without modifying output. No backend or owner needed. */
int mv_text_aligned_x(int x, int width, int alignment, int *out);

/* Per-process drawing commands. Append copies values and text in call order.
 * Box dimensions and circle radius must be positive; filled is 0 or 1. Box
 * far corners and circle bounds must fit int. Text is a NUL-terminated string;
 * size is passed unchanged to the built-in renderer (raylib uses a minimum of
 * 10 pixels, including for nonpositive values).
 * Commands use world coordinates, not the graph's position/rotation/scale.
 * They do not change graph selection or collision geometry. They survive ticks,
 * sleep and freeze until explicit clear or process cleanup. Language hosts clear
 * immediately before actual activation, including low-cadence activations.
 * Count writes only on success. Clear is idempotent. Owner-thread only.
 * Invalid/foreign arguments return MV_INVALID_ARGUMENT; dead processes return
 * MV_NOT_FOUND. Allocation failure returns MV_OUT_OF_MEMORY. Failure preserves
 * the previous list and caller ownership. No backend is needed for collection. */
MvResult mv_process_draw_box(MvContext *ctx, MvProcessRef process,
    int x, int y, int width, int height, int filled, MvColor color);
MvResult mv_process_draw_circle(MvContext *ctx, MvProcessRef process,
    int x, int y, int radius, int filled, MvColor color);
MvResult mv_process_draw_text(MvContext *ctx, MvProcessRef process,
    const char *text, int x, int y, int size, MvColor color);
MvResult mv_process_draw_count(MvContext *ctx, MvProcessRef process, size_t *out);
MvResult mv_process_draw_clear(MvContext *ctx, MvProcessRef process);

/* Backend-independent graphic geometry. Dimensions must be finite and in
 * FLT_MIN..FLT_MAX/16. Rectangles accept center (top_left=0) or top-left (1)
 * pivots. Circles are centered. Return 1 on success, 0 for invalid input.
 * Failure leaves out unchanged. Values own no resources and can be copied. */
int mv_graphic_rect(double width, double height, int top_left, MvCollisionShape *out);
int mv_graphic_circle(double radius, MvCollisionShape *out);
/* Copied local sprite rectangle from positive pixel dimensions and a pivot in
 * cropped-image coordinates. Pivots must be finite with magnitude at most FLT_MAX/16, including
 * outside the rectangle. Original-canvas trim adjustment belongs to the importer.
 * Return 1, or 0 without changing out for invalid pivots, NULL or nonpositive dimensions.
 * No image, backend, process or resource is required or retained. */
int mv_graphic_sprite(int width, int height, double pivot_x, double pivot_y, MvCollisionShape *out);
#ifdef __cplusplus
}
#endif
#endif
