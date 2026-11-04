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

#ifndef MALVADO_COLLISION_H
#define MALVADO_COLLISION_H

#include <stddef.h>
#include "malvado/malvado.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { double x, y, width, height; } MvRect;

/* Local geometry is centered at (offset_x, offset_y). Rectangles use width
 * and height; circles use radius. The transform scales and rotates the offset
 * around the origin before translating it into world space. */
typedef enum { MV_COLLISION_CIRCLE, MV_COLLISION_RECT } MvCollisionKind;
typedef struct {
    MvCollisionKind kind;
    double width, height, radius;
    double offset_x, offset_y;
} MvCollisionShape;
typedef struct { double x, y, angle, scale; } MvCollisionTransform;

/* Return 1 for overlap, 0 for separation or tangency, and -1 for invalid
 * geometry or transform. World centers and bounding half-extents must not exceed
 * DBL_MAX / 16 in magnitude. Rotation is clockwise in screen space. */
int mv_collision_overlap(MvCollisionShape a, MvCollisionTransform transform_a,
                         MvCollisionShape b, MvCollisionTransform transform_b);

/* Process colliders are copied into the context and removed with the process.
 * Set again after changing a process transform. Clear is idempotent. Dead or
 * foreign references return an error. These functions do not allocate. */
MvResult mv_process_collider_set(MvContext *ctx, MvProcessRef process,
                                 MvCollisionShape shape, MvCollisionTransform transform);
MvResult mv_process_collider_clear(MvContext *ctx, MvProcessRef process);
/* Compare two live processes; self-comparison and missing colliders return 0.
 * On error, the output is unchanged. */
MvResult mv_process_collider_test(MvContext *ctx, MvProcessRef a, MvProcessRef b, int *overlap);
/* Copy up to capacity colliding process references in creation order. Type 0
 * selects all types. total receives the full count. A missing source collider
 * returns no results. Sleeping and frozen processes are included. */
MvResult mv_process_collider_find(MvContext *ctx, MvProcessRef source, uint64_t type,
                                 MvProcessRef *out, size_t capacity, size_t *total);

/* A world owns its rectangles. IDs are one-based and follow insertion order.
 * Create returns NULL on allocation failure; destroy accepts NULL. */
typedef struct MvCollisionWorld MvCollisionWorld;
MvCollisionWorld *mv_collision_world_create(void);
void mv_collision_world_destroy(MvCollisionWorld *world);
/* Return 1 and write the ID, or 0 on invalid input, allocation failure, or
 * if the world already contains INT_MAX rectangles.
 * On failure, the world and output are unchanged. */
int mv_collision_world_add_rect(MvCollisionWorld *world, MvRect rect, size_t *id);
/* Return 1 and write the first overlapping ID, 0 for none, or -1 on invalid
 * input. Output is unchanged when no hit or on error. */
int mv_collision_world_query(const MvCollisionWorld *world, MvCollisionShape shape,
                             MvCollisionTransform transform, size_t *id);

typedef struct {
    int hit;
    size_t id;
    double fraction, normal_x, normal_y;
} MvCollisionSweep;
/* Sweep a circle or rectangle against world rectangles. Returns 1 on success, -1 on
 * invalid inputs, unsupported shapes, initial overlap, or insufficient numeric
 * precision for a circle against a candidate. On failure the
 * output is unchanged. A hit fraction is in [0,1]; without a hit it is 1. */
int mv_collision_world_sweep(const MvCollisionWorld *world, MvCollisionShape shape,
                             MvCollisionTransform transform, double dx, double dy,
                             MvCollisionSweep *out);

/* Move a shape by the permitted fraction. Return 1 on success, -1 on error.
 * The transform and result are unchanged on error. A hit fraction can be
 * reduced by a few representable steps to avoid numerical overlap. */
int mv_collision_world_move(const MvCollisionWorld *world, MvCollisionShape shape,
                            MvCollisionTransform *transform, double dx, double dy,
                            MvCollisionSweep *out);

/* Return 1 for intersection, 0 for none, or -1 for invalid input. */
int mv_rect_overlaps(MvRect a, MvRect b);
int mv_rect_contains(MvRect rect, double x, double y);

#ifdef __cplusplus
}
#endif
#endif
