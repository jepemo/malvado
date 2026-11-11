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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include "malvado/graphic.h"
#include <assert.h>
#include <limits.h>
#include <string.h>
int main(void)
{
    MvCollisionShape shape;
    assert(mv_graphic_sprite(4, 8, 2, 4, &shape));
    assert(shape.offset_x == 0 && shape.offset_y == 0 && shape.width == 4 && shape.height == 8);
    assert(mv_graphic_sprite(5, 7, 0, 0, &shape));
    assert(shape.offset_x == 2.5 && shape.offset_y == 3.5);
    assert(mv_graphic_sprite(4, 4, -2, 7, &shape));
    assert(shape.offset_x == 4 && shape.offset_y == -5);
    MvCollisionShape dot;
    assert(mv_graphic_circle(0.5, &dot));
    MvCollisionTransform rotated = {.x = 20, .y = 20, .angle = 90, .scale = 2};
    assert(mv_collision_overlap(shape, rotated, dot, (MvCollisionTransform){.x=30, .y=28, .scale=1}) == 1);
    assert(mv_collision_overlap(shape, rotated, dot, (MvCollisionTransform){.x=20, .y=20, .scale=1}) == 0);
    assert(mv_graphic_sprite(INT_MAX, INT_MAX, INT_MIN, INT_MAX, &shape));
    assert(shape.offset_x == INT_MAX / 2.0 - (double)INT_MIN);
    MvCollisionShape before;
    memcpy(&before, &shape, sizeof(shape));
    assert(!mv_graphic_sprite(0, 1, 0, 0, &shape));
    assert(!mv_graphic_sprite(1, -1, 0, 0, &shape));
    assert(!mv_graphic_sprite(1, 1, 0, 0, NULL));
    assert(!memcmp(&shape, &before, sizeof(shape)));
    return 0;
}
