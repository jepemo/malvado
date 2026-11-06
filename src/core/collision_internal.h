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

#ifndef MALVADO_COLLISION_INTERNAL_H
#define MALVADO_COLLISION_INTERNAL_H

#include "malvado/collision.h"

typedef struct {
    double x, y, half_width, half_height;
} MvCollisionBounds;

int mv_collision_bounds(MvCollisionShape shape, MvCollisionTransform transform,
                        MvCollisionBounds *bounds);

#endif
