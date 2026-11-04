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

#include <assert.h>
#include <float.h>
#include <math.h>
#include "malvado/collision.h"

int
main(void)
{
    MvRect a = {0, 0, 10, 10};
    assert(mv_rect_overlaps(a, (MvRect){9, 9, 2, 2}) == 1);
    assert(mv_rect_overlaps(a, (MvRect){10, 0, 1, 1}) == 0);
    assert(mv_rect_overlaps(a, (MvRect){-2, -2, 1, 1}) == 0);
    assert(mv_rect_contains(a, 0, 0) == 1);
    assert(mv_rect_contains(a, 10, 5) == 0);
    assert(mv_rect_contains(a, 5, 10) == 0);
    assert(mv_rect_contains(a, NAN, 0) == -1);
    assert(mv_rect_overlaps(a, (MvRect){0, 0, 0, 1}) == -1);
    assert(mv_rect_overlaps(a, (MvRect){DBL_MAX, 0, DBL_MAX, 1}) == -1);
    assert(mv_rect_overlaps(a, (MvRect){DBL_MAX, 0, 1, 1}) == -1);
    MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionShape rect = {.kind = MV_COLLISION_RECT, .width = 4, .height = 2};
    MvCollisionTransform origin = {0, 0, 0, 1};
    assert(mv_collision_overlap(circle, origin, circle, (MvCollisionTransform){1, 0, 0, 1}) == 1);
    assert(mv_collision_overlap(circle, origin, circle, (MvCollisionTransform){2, 0, 0, 1}) == 0);
    assert(mv_collision_overlap(circle, (MvCollisionTransform){2.5, 0, 0, 1}, rect, origin) == 1);
    assert(mv_collision_overlap(circle, (MvCollisionTransform){3, 0, 0, 1}, rect, origin) == 0);
    assert(mv_collision_overlap(circle, (MvCollisionTransform){2.8, 1.8, 0, 1}, rect, origin) == 0);
    assert(mv_collision_overlap(rect, origin, rect, (MvCollisionTransform){3, 0, 0, 1}) == 1);
    assert(mv_collision_overlap(rect, origin, rect, (MvCollisionTransform){3, 0, 90, 1}) == 0);
    assert(mv_collision_overlap(rect, origin, rect, (MvCollisionTransform){2.5, 0, 90, 1}) == 1);
    MvCollisionShape top_left = {.kind = MV_COLLISION_RECT, .width = 4, .height = 2,
                                  .offset_x = 2, .offset_y = 1};
    assert(mv_collision_overlap(top_left, (MvCollisionTransform){10, 10, 0, 1},
                                circle, (MvCollisionTransform){10.5, 10.5, 0, 1}) == 1);
    assert(mv_collision_overlap(top_left, (MvCollisionTransform){10, 10, 0, 1},
                                circle, (MvCollisionTransform){8, 10, 0, 1}) == 0);
    circle.offset_x = 2;
    assert(mv_collision_overlap(circle, (MvCollisionTransform){0, 0, 90, 1},
                                circle, (MvCollisionTransform){0, 4, -90, 1}) == 1);
    circle.offset_x = 0;
    assert(mv_collision_overlap(circle, origin, circle, (MvCollisionTransform){2.5, 0, 0, 2}) == 1);
    assert(mv_collision_overlap(circle, origin, rect, (MvCollisionTransform){0, 0, NAN, 1}) == -1);
    assert(mv_collision_overlap(circle, origin, rect, (MvCollisionTransform){0, 0, 0, 0}) == -1);
    assert(mv_collision_overlap(rect, (MvCollisionTransform){0, 0, 45, 1},
                                circle, (MvCollisionTransform){0, 1.8, 0, 1}) == 1);
    assert(mv_collision_overlap(circle, (MvCollisionTransform){2.8, 1.8, 0, 1}, rect, origin) ==
           mv_collision_overlap(rect, origin, circle, (MvCollisionTransform){2.8, 1.8, 0, 1}));
    assert(mv_collision_overlap((MvCollisionShape){.kind = MV_COLLISION_RECT, .width = 0, .height = 1},
                                origin, circle, origin) == -1);
    assert(mv_collision_overlap((MvCollisionShape){.kind = MV_COLLISION_CIRCLE, .radius = INFINITY},
                                origin, circle, origin) == -1);
    assert(mv_collision_overlap(circle, (MvCollisionTransform){DBL_MAX, 0, 0, 1}, rect, origin) == -1);
    assert(mv_collision_overlap((MvCollisionShape){.kind = MV_COLLISION_RECT, .width = DBL_MIN, .height = 1},
                                (MvCollisionTransform){0, 0, 45, DBL_MIN}, circle, origin) == -1);
    return 0;
}
