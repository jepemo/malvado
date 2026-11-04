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

#include <float.h>
#include <math.h>
#include "malvado/collision.h"

static int
valid_rect(MvRect r)
{
    return isfinite(r.x) && isfinite(r.y) && isfinite(r.width) &&
           isfinite(r.height) && r.width > 0 && r.height > 0 &&
           isfinite(r.x + r.width) && isfinite(r.y + r.height) &&
           r.x + r.width > r.x && r.y + r.height > r.y;
}

int
mv_rect_overlaps(MvRect a, MvRect b)
{
    if (!valid_rect(a) || !valid_rect(b))
        return -1;
    return a.x < b.x + b.width && b.x < a.x + a.width &&
           a.y < b.y + b.height && b.y < a.y + a.height;
}

int
mv_rect_contains(MvRect r, double x, double y)
{
    if (!valid_rect(r) || !isfinite(x) || !isfinite(y))
        return -1;
    return x >= r.x && x < r.x + r.width &&
           y >= r.y && y < r.y + r.height;
}

typedef struct {
    MvCollisionKind kind;
    double x, y, ux, uy, hx, hy, radius, bound_x, bound_y;
} WorldShape;

static int
world_shape(MvCollisionShape shape, MvCollisionTransform transform, WorldShape *out)
{
    if ((shape.kind != MV_COLLISION_CIRCLE && shape.kind != MV_COLLISION_RECT) ||
        !isfinite(shape.offset_x) || !isfinite(shape.offset_y) ||
        !isfinite(transform.x) || !isfinite(transform.y) || !isfinite(transform.angle) ||
        !isfinite(transform.scale) || transform.scale <= 0)
        return 0;
    if (shape.kind == MV_COLLISION_CIRCLE) {
        if (!isfinite(shape.radius) || shape.radius <= 0)
            return 0;
    } else if (!isfinite(shape.width) || !isfinite(shape.height) ||
               shape.width <= 0 || shape.height <= 0) {
        return 0;
    }
    double angle = fmod(transform.angle, 360.0) * (3.14159265358979323846 / 180.0);
    double c = cos(angle), s = sin(angle);
    double ox = shape.offset_x * transform.scale, oy = shape.offset_y * transform.scale;
    *out = (WorldShape){.kind = shape.kind,
        .x = transform.x + c * ox - s * oy,
        .y = transform.y + s * ox + c * oy,
        .ux = c, .uy = s};
    if (shape.kind == MV_COLLISION_CIRCLE) {
        out->radius = shape.radius * transform.scale;
        out->bound_x = out->bound_y = out->radius;
    } else {
        out->hx = shape.width * transform.scale / 2;
        out->hy = shape.height * transform.scale / 2;
        out->bound_x = fabs(c) * out->hx + fabs(s) * out->hy;
        out->bound_y = fabs(s) * out->hx + fabs(c) * out->hy;
    }
    return isfinite(out->x) && isfinite(out->y) &&
           fabs(out->x) <= DBL_MAX / 16 && fabs(out->y) <= DBL_MAX / 16 &&
           out->bound_x <= DBL_MAX / 16 && out->bound_y <= DBL_MAX / 16 &&
           isfinite(out->bound_x) &&
           isfinite(out->bound_y) && out->bound_x > 0 && out->bound_y > 0 &&
           isfinite(out->x + out->bound_x) && isfinite(out->x - out->bound_x) &&
           isfinite(out->y + out->bound_y) && isfinite(out->y - out->bound_y);
}

static double
projection_radius(const WorldShape *shape, double x, double y)
{
    return shape->hx * fabs(shape->ux * x + shape->uy * y) +
           shape->hy * fabs(-shape->uy * x + shape->ux * y);
}

static int
rectangles_overlap(const WorldShape *a, const WorldShape *b)
{
    double dx = b->x - a->x, dy = b->y - a->y;
    const double axes[4][2] = {{a->ux, a->uy}, {-a->uy, a->ux},
                                {b->ux, b->uy}, {-b->uy, b->ux}};
    for (int i = 0; i < 4; ++i) {
        double x = axes[i][0], y = axes[i][1];
        if (fabs(dx * x + dy * y) >= projection_radius(a, x, y) + projection_radius(b, x, y))
            return 0;
    }
    return 1;
}

static int
circle_rect_overlap(const WorldShape *circle, const WorldShape *rect)
{
    double dx = circle->x - rect->x, dy = circle->y - rect->y;
    double x = fabs(dx * rect->ux + dy * rect->uy) - rect->hx;
    double y = fabs(-dx * rect->uy + dy * rect->ux) - rect->hy;
    x = fmax(x, 0);
    y = fmax(y, 0);
    return hypot(x, y) < circle->radius;
}

int
mv_collision_overlap(MvCollisionShape a, MvCollisionTransform transform_a,
                     MvCollisionShape b, MvCollisionTransform transform_b)
{
    WorldShape first, second;
    if (!world_shape(a, transform_a, &first) || !world_shape(b, transform_b, &second))
        return -1;
    double dx = second.x - first.x, dy = second.y - first.y;
    if (fabs(dx) >= first.bound_x + second.bound_x ||
        fabs(dy) >= first.bound_y + second.bound_y)
        return 0;
    if (first.kind == MV_COLLISION_CIRCLE && second.kind == MV_COLLISION_CIRCLE)
        return hypot(dx, dy) < first.radius + second.radius;
    if (first.kind == MV_COLLISION_CIRCLE)
        return circle_rect_overlap(&first, &second);
    if (second.kind == MV_COLLISION_CIRCLE)
        return circle_rect_overlap(&second, &first);
    return rectangles_overlap(&first, &second);
}
