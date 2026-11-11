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

/* New processes derive colliders from their native primitive graph and current
 * x/y, angle and scale. No graph means no collider. Bind a copied local override
 * to use another shape with those properties; NULL restores graph selection.
 * Binding validates current selected geometry/transform before committing.
 * Fixed set below instead copies an independent shape/transform. Re-set a fixed
 * collider to move it. Clear disables all selection until bind or set.
 * Failed changes preserve the previous selection. Dead/foreign references are
 * errors. Owner-thread only, no allocation or retained pointers. */
MvResult mv_process_collider_bind(MvContext *ctx, MvProcessRef process,
                                  const MvCollisionShape *override);
MvResult mv_process_collider_set(MvContext *ctx, MvProcessRef process,
                                 MvCollisionShape shape, MvCollisionTransform transform);
MvResult mv_process_collider_clear(MvContext *ctx, MvProcessRef process);
/* Compare two live processes; self-comparison and missing colliders return 0.
 * Invalid bound source/target transforms return MV_INVALID_ARGUMENT without
 * changing output. Bound transforms refresh on queries, never on drawing. */
MvResult mv_process_collider_test(MvContext *ctx, MvProcessRef a, MvProcessRef b, int *overlap);
/* Find the first collision in creation order, or write an all-zero reference
 * if none exists. Type 0 selects all types. No allocation is performed. */
MvResult mv_process_collider_first(MvContext *ctx, MvProcessRef source, uint64_t type,
                                  MvProcessRef *out);
/* Copy up to capacity colliding process references in creation order. Type 0
 * selects all types. total receives the full count. A missing source collider
 * returns no results. Sleeping and frozen processes are included. Invalid bound
 * sources fail without changing outputs; invalid bound candidates are skipped. */
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

/* Contextual world operations use the live process x/y, angle and scale.
 * shape is borrowed for this call; NULL means no collider. Query writes a
 * one-based world ID or 0. Move atomically commits x/y and the sweep result.
 * With no shape, move applies the full displacement and returns no hit.
 * dx/dy must be finite and at most DBL_MAX/16 in magnitude. Geometry, numeric
 * limits and initial overlap follow the world operations above. Invalid input
 * returns MV_INVALID_ARGUMENT; dead processes return MV_NOT_FOUND. Failure
 * preserves position and outputs. Foreign references are invalid. Owner-thread
 * only; no allocation, callbacks, ownership transfer or process activation.
 * These calls do not change explicitly registered process colliders. */
MvResult mv_process_world_query(MvContext *ctx, MvProcessRef process,
                                const MvCollisionWorld *world,
                                const MvCollisionShape *shape, size_t *out);
MvResult mv_process_world_move(MvContext *ctx, MvProcessRef process,
                               const MvCollisionWorld *world,
                               const MvCollisionShape *shape, double dx, double dy,
                               MvCollisionSweep *out);

/* Return 1 for intersection, 0 for none, or -1 for invalid input. */
int mv_rect_overlaps(MvRect a, MvRect b);
int mv_rect_contains(MvRect rect, double x, double y);

#ifdef __cplusplus
}
#endif
#endif
