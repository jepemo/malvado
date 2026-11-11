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
#include <assert.h>
#include <float.h>
#include <math.h>
#include "malvado/graphic.h"
int main(void)
{
    MvCollisionShape shape = {0};
    assert(mv_graphic_rect(20, 10, 1, &shape));
    assert(shape.offset_x == 10 && shape.offset_y == 5);
    assert(mv_graphic_rect(20, 10, 0, &shape));
    assert(shape.offset_x == 0 && shape.offset_y == 0);
    double invalid[] = {0, -1, NAN, INFINITY, DBL_MAX, FLT_MIN / 2.0};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(*invalid); ++i) {
        assert(!mv_graphic_rect(invalid[i], 10, 0, &shape));
        assert(!mv_graphic_rect(10, invalid[i], 0, &shape));
        assert(!mv_graphic_circle(invalid[i], &shape));
        assert(shape.width == 20 && shape.height == 10);
    }
    assert(!mv_graphic_rect(1, 1, 2, &shape));
    assert(!mv_graphic_rect(1, 1, 0, NULL));
    assert(!mv_graphic_circle(1, NULL));
    assert(mv_graphic_circle(FLT_MIN, &shape));
    assert(mv_graphic_circle(FLT_MAX / 16, &shape));
    assert(shape.kind == MV_COLLISION_CIRCLE && shape.width == 0);
    return 0;
}
