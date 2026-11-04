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
    return 0;
}
