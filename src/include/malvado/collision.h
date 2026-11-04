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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { double x, y, width, height; } MvRect;

/* Return 1 for intersection, 0 for none, or -1 for invalid input. */
int mv_rect_overlaps(MvRect a, MvRect b);
int mv_rect_contains(MvRect rect, double x, double y);

#ifdef __cplusplus
}
#endif
#endif
