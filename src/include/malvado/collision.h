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
/* Sweep a rectangle against world rectangles. Returns 1 on success, -1 on
 * invalid inputs, unsupported shapes, or initial overlap. On failure the
 * output is unchanged. A hit fraction is in [0,1]; without a hit it is 1. */
int mv_collision_world_sweep(const MvCollisionWorld *world, MvCollisionShape shape,
                             MvCollisionTransform transform, double dx, double dy,
                             MvCollisionSweep *out);

/* Return 1 for intersection, 0 for none, or -1 for invalid input. */
int mv_rect_overlaps(MvRect a, MvRect b);
int mv_rect_contains(MvRect rect, double x, double y);

#ifdef __cplusplus
}
#endif
#endif
