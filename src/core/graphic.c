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

#include "malvado/graphic.h"
#include <float.h>
#include <math.h>

static int
valid_dimension(double value)
{
    return isfinite(value) && value >= FLT_MIN && value <= FLT_MAX / 16;
}

int
mv_graphic_rect(double width, double height, int top_left, MvCollisionShape *out)
{
    if (!out || !valid_dimension(width) || !valid_dimension(height) ||
        (top_left != 0 && top_left != 1))
        return 0;
    *out = (MvCollisionShape){.kind = MV_COLLISION_RECT, .width = width, .height = height,
        .offset_x = top_left ? width / 2 : 0, .offset_y = top_left ? height / 2 : 0};
    return 1;
}

int
mv_graphic_circle(double radius, MvCollisionShape *out)
{
    if (!out || !valid_dimension(radius))
        return 0;
    *out = (MvCollisionShape){.kind = MV_COLLISION_CIRCLE, .radius = radius};
    return 1;
}

int
mv_graphic_sprite(int width, int height, int pivot_x, int pivot_y, MvCollisionShape *out)
{
    if (!out || width <= 0 || height <= 0) return 0;
    *out = (MvCollisionShape){.kind = MV_COLLISION_RECT, .width = width, .height = height,
        .offset_x = width / 2.0 - pivot_x, .offset_y = height / 2.0 - pivot_y};
    return 1;
}
