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

#ifndef MALVADO_GRAPHIC_H
#define MALVADO_GRAPHIC_H
#include "malvado/collision.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Backend-independent graphic geometry. Dimensions must be finite and in
 * FLT_MIN..FLT_MAX/16. Rectangles accept center (top_left=0) or top-left (1)
 * pivots. Circles are centered. Return 1 on success, 0 for invalid input.
 * Failure leaves out unchanged. Values own no resources and can be copied. */
int mv_graphic_rect(double width, double height, int top_left, MvCollisionShape *out);
int mv_graphic_circle(double radius, MvCollisionShape *out);
#ifdef __cplusplus
}
#endif
#endif
