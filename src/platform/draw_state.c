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

#include "malvado/platform.h"

static int
valid_channel(int64_t channel)
{
    return channel >= 0 && channel <= 255;
}

int
mv_draw_state_init(MvDrawState *state)
{
    if (!state)
        return 0;
    *state = (MvDrawState){{255, 255, 255, 255}, {0, 0, 0, 255}};
    return 1;
}

int
mv_draw_state_color(MvDrawState *state, int64_t r, int64_t g, int64_t b, int64_t a)
{
    if (!state || !valid_channel(r) || !valid_channel(g) || !valid_channel(b) || !valid_channel(a))
        return 0;
    state->color = (MvColor){r, g, b, a};
    return 1;
}

int
mv_draw_state_background(MvDrawState *state, int64_t r, int64_t g, int64_t b)
{
    if (!state || !valid_channel(r) || !valid_channel(g) || !valid_channel(b))
        return 0;
    state->background = (MvColor){r, g, b, 255};
    return 1;
}
