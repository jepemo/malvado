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
