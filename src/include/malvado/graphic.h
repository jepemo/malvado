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
 * primitive by default. Image/animation graphs are not stored
 * by this primitive service. */
MvResult mv_process_graphic_set(MvContext *ctx, MvProcessRef process, MvGraphic graphic);
MvResult mv_process_graphic_get(MvContext *ctx, MvProcessRef process, MvGraphic *graphic, int *present);
MvResult mv_process_graphic_clear(MvContext *ctx, MvProcessRef process);

/* Backend-independent graphic geometry. Dimensions must be finite and in
 * FLT_MIN..FLT_MAX/16. Rectangles accept center (top_left=0) or top-left (1)
 * pivots. Circles are centered. Return 1 on success, 0 for invalid input.
 * Failure leaves out unchanged. Values own no resources and can be copied. */
int mv_graphic_rect(double width, double height, int top_left, MvCollisionShape *out);
int mv_graphic_circle(double radius, MvCollisionShape *out);
/* Copied local sprite rectangle from positive pixel dimensions and a pivot in
 * cropped-image coordinates. Pivots can use the full signed int range, including
 * outside the rectangle. Original-canvas trim adjustment belongs to the importer.
 * Return 1, or 0 without changing out for NULL/nonpositive dimensions.
 * No image, backend, process or resource is required or retained. */
int mv_graphic_sprite(int width, int height, int pivot_x, int pivot_y, MvCollisionShape *out);
#ifdef __cplusplus
}
#endif
#endif
