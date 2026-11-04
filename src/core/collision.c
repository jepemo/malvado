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
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
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
    if (shape.kind == MV_COLLISION_RECT && (out->hx <= 0 || out->hy <= 0))
        return 0;
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

typedef struct {
    MvRect rect;
    size_t id;
} WorldRect;

struct MvCollisionWorld {
    WorldRect *rects;
    size_t count, capacity;
};

MvCollisionWorld *
mv_collision_world_create(void)
{
    return calloc(1, sizeof(MvCollisionWorld));
}

void
mv_collision_world_destroy(MvCollisionWorld *world)
{
    if (!world)
        return;
    free(world->rects);
    free(world);
}

int
mv_collision_world_add_rect(MvCollisionWorld *world, MvRect rect, size_t *id)
{
    if (!world || !id || !valid_rect(rect) || world->count >= INT_MAX)
        return 0;
    MvCollisionShape shape = {.kind = MV_COLLISION_RECT, .width = rect.width, .height = rect.height};
    MvCollisionTransform transform = {rect.x + rect.width / 2, rect.y + rect.height / 2, 0, 1};
    WorldShape bounds;
    if (!world_shape(shape, transform, &bounds))
        return 0;
    if (world->count == world->capacity) {
        size_t capacity = world->capacity ? world->capacity * 2 : 16;
        if (capacity < world->capacity || capacity > SIZE_MAX / sizeof(*world->rects))
            return 0;
        WorldRect *rects = realloc(world->rects, capacity * sizeof(*rects));
        if (!rects)
            return 0;
        world->rects = rects;
        world->capacity = capacity;
    }
    size_t position = world->count;
    while (position && world->rects[position - 1].rect.x > rect.x) {
        world->rects[position] = world->rects[position - 1];
        --position;
    }
    world->rects[position] = (WorldRect){rect, world->count + 1};
    *id = ++world->count;
    return 1;
}

int
mv_collision_world_query(const MvCollisionWorld *world, MvCollisionShape shape,
                         MvCollisionTransform transform, size_t *id)
{
    WorldShape bounds;
    if (!world || !id || !world_shape(shape, transform, &bounds))
        return -1;
    size_t first = SIZE_MAX;
    for (size_t i = 0; i < world->count; ++i) {
        WorldRect item = world->rects[i];
        if (item.rect.x >= bounds.x + bounds.bound_x)
            break;
        if (item.rect.x + item.rect.width <= bounds.x - bounds.bound_x ||
            item.rect.y >= bounds.y + bounds.bound_y ||
            item.rect.y + item.rect.height <= bounds.y - bounds.bound_y)
            continue;
        MvCollisionShape obstacle = {.kind = MV_COLLISION_RECT, .width = item.rect.width,
                                     .height = item.rect.height};
        MvCollisionTransform location = {item.rect.x + item.rect.width / 2,
                                         item.rect.y + item.rect.height / 2, 0, 1};
        if (mv_collision_overlap(shape, transform, obstacle, location) == 1 && item.id < first)
            first = item.id;
    }
    if (first == SIZE_MAX)
        return 0;
    *id = first;
    return 1;
}

static int
sweep_rectangles(const WorldShape *moving, const WorldShape *obstacle,
                 double dx, double dy, double *time, double *normal_x, double *normal_y)
{
    const double axes[4][2] = {{moving->ux, moving->uy}, {-moving->uy, moving->ux},
                                {1, 0}, {0, 1}};
    double enter = -INFINITY, exit = INFINITY, nx = 0, ny = 0;
    for (int i = 0; i < 4; ++i) {
        double x = axes[i][0], y = axes[i][1];
        double distance = (moving->x - obstacle->x) * x + (moving->y - obstacle->y) * y;
        double radius = projection_radius(moving, x, y) + projection_radius(obstacle, x, y);
        double velocity = dx * x + dy * y;
        if (velocity == 0) {
            if (fabs(distance) >= radius)
                return 0;
            continue;
        }
        double first = (-radius - distance) / velocity;
        double last = (radius - distance) / velocity;
        if (first > last) {
            double swap = first;
            first = last;
            last = swap;
        }
        if (first > enter) {
            enter = first;
            nx = velocity > 0 ? -x : x;
            ny = velocity > 0 ? -y : y;
        }
        if (last < exit)
            exit = last;
        if (enter >= exit)
            return 0;
    }
    if (enter < 0 || enter > 1 || exit <= 0)
        return 0;
    *time = enter;
    *normal_x = nx;
    *normal_y = ny;
    return 1;
}

static void
sweep_circle_face(double time, double along, double low, double high,
                  double nx, double ny, double *best, double *normal_x, double *normal_y)
{
    if (time >= 0 && time <= 1 && time < *best && along >= low && along <= high) {
        *best = time;
        *normal_x = nx;
        *normal_y = ny;
    }
}

static int
sweep_circle_rect(const WorldShape *circle, MvRect rect, double dx, double dy,
                  double *time, double *normal_x, double *normal_y)
{
    double best = INFINITY, nx = 0, ny = 0;
    double right = rect.x + rect.width, bottom = rect.y + rect.height;
    if ((dy == 0 && (circle->y == rect.y - circle->radius ||
                     circle->y == bottom + circle->radius)) ||
        (dx == 0 && (circle->x == rect.x - circle->radius ||
                     circle->x == right + circle->radius)))
        return 0;
    if (dx > 0) {
        double t = (rect.x - circle->radius - circle->x) / dx;
        sweep_circle_face(t, circle->y + t * dy, rect.y, bottom, -1, 0, &best, &nx, &ny);
    } else if (dx < 0) {
        double t = (right + circle->radius - circle->x) / dx;
        sweep_circle_face(t, circle->y + t * dy, rect.y, bottom, 1, 0, &best, &nx, &ny);
    }
    if (dy > 0) {
        double t = (rect.y - circle->radius - circle->y) / dy;
        sweep_circle_face(t, circle->x + t * dx, rect.x, right, 0, -1, &best, &nx, &ny);
    } else if (dy < 0) {
        double t = (bottom + circle->radius - circle->y) / dy;
        sweep_circle_face(t, circle->x + t * dx, rect.x, right, 0, 1, &best, &nx, &ny);
    }
    const double corners[4][2] = {{rect.x, rect.y}, {right, rect.y},
                                  {rect.x, bottom}, {right, bottom}};
    for (int i = 0; i < 4; ++i) {
        double x = circle->x - corners[i][0], y = circle->y - corners[i][1];
        double scale = fmax(circle->radius, fmax(fmax(fabs(x), fabs(y)), fmax(fabs(dx), fabs(dy))));
        double px = x / scale, py = y / scale, vx = dx / scale, vy = dy / scale;
        double radius = circle->radius / scale;
        double a = vx * vx + vy * vy;
        double b = px * vx + py * vy;
        double c = px * px + py * py - radius * radius;
        double discriminant = b * b - a * c;
        if (a == 0 || b >= 0 || discriminant <= 0)
            continue;
        double t = c / (-b + sqrt(discriminant));
        if (t < 0 || t > 1 || t >= best)
            continue;
        double hit_x = x + t * dx, hit_y = y + t * dy;
        if ((i % 2 == 0 ? hit_x > 0 : hit_x < 0) ||
            (i < 2 ? hit_y > 0 : hit_y < 0))
            continue;
        double length = hypot(hit_x, hit_y);
        if (length == 0)
            continue;
        best = t;
        nx = hit_x / length;
        ny = hit_y / length;
    }
    if (!isfinite(best))
        return 0;
    *time = best;
    *normal_x = nx;
    *normal_y = ny;
    return 1;
}

int
mv_collision_world_sweep(const MvCollisionWorld *world, MvCollisionShape shape,
                         MvCollisionTransform transform, double dx, double dy,
                         MvCollisionSweep *out)
{
    WorldShape start, end;
    if (!world || !out || !isfinite(dx) || !isfinite(dy) ||
        fabs(dx) > DBL_MAX / 16 || fabs(dy) > DBL_MAX / 16 ||
        !world_shape(shape, transform, &start))
        return -1;
    MvCollisionTransform destination = transform;
    destination.x += dx;
    destination.y += dy;
    if (!world_shape(shape, destination, &end))
        return -1;
    size_t overlapping;
    int initial = mv_collision_world_query(world, shape, transform, &overlapping);
    if (initial != 0)
        return -1;
    MvCollisionSweep result = {.fraction = 1};
    double left = fmin(start.x, end.x) - start.bound_x;
    double right = fmax(start.x, end.x) + start.bound_x;
    double top = fmin(start.y, end.y) - start.bound_y;
    double bottom = fmax(start.y, end.y) + start.bound_y;
    for (size_t i = 0; i < world->count; ++i) {
        WorldRect item = world->rects[i];
        if (item.rect.x > right)
            break;
        if (item.rect.x + item.rect.width < left || item.rect.y > bottom ||
            item.rect.y + item.rect.height < top)
            continue;
        MvCollisionShape obstacle_shape = {.kind = MV_COLLISION_RECT, .width = item.rect.width,
                                          .height = item.rect.height};
        MvCollisionTransform obstacle_transform = {item.rect.x + item.rect.width / 2,
                                                   item.rect.y + item.rect.height / 2, 0, 1};
        WorldShape obstacle;
        if (!world_shape(obstacle_shape, obstacle_transform, &obstacle))
            return -1;
        double time, nx, ny;
        if (shape.kind == MV_COLLISION_CIRCLE) {
            double range = fmax(fmax(fabs(dx), fabs(dy)),
                                fmax(fmax(fabs(start.x - item.rect.x), fabs(start.y - item.rect.y)),
                                     fmax(item.rect.width, item.rect.height)));
            if (range > 0 && start.radius / range < 8 * sqrt(DBL_EPSILON))
                return -1;
        }
        int hit = shape.kind == MV_COLLISION_CIRCLE ?
                  sweep_circle_rect(&start, item.rect, dx, dy, &time, &nx, &ny) :
                  sweep_rectangles(&start, &obstacle, dx, dy, &time, &nx, &ny);
        if (hit &&
            (!result.hit || time < result.fraction || (time == result.fraction && item.id < result.id))) {
            result = (MvCollisionSweep){1, item.id, time, nx, ny};
        }
    }
    *out = result;
    return 1;
}

int
mv_collision_world_move(const MvCollisionWorld *world, MvCollisionShape shape,
                        MvCollisionTransform *transform, double dx, double dy,
                        MvCollisionSweep *out)
{
    if (!transform || !out)
        return -1;
    MvCollisionSweep result;
    if (mv_collision_world_sweep(world, shape, *transform, dx, dy, &result) < 0)
        return -1;
    MvCollisionTransform position = *transform;
    for (int attempt = 0; attempt < 8; ++attempt) {
        position.x = transform->x + dx * result.fraction;
        position.y = transform->y + dy * result.fraction;
        size_t hit;
        int overlapping = mv_collision_world_query(world, shape, position, &hit);
        if (overlapping == 0) {
            *transform = position;
            *out = result;
            return 1;
        }
        if (overlapping < 0 || !result.hit || result.fraction <= 0)
            return -1;
        result.fraction = nextafter(result.fraction, 0);
    }
    return -1;
}
