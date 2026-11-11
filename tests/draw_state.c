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
#include <string.h>
#include "malvado/platform.h"
int main(void)
{
    MvDrawState state, before;
    assert(!mv_draw_state_init(NULL));
    assert(mv_draw_state_init(&state));
    assert(state.color.r == 255 && state.color.a == 255);
    assert(state.background.r == 0 && state.background.a == 255);
    assert(mv_draw_state_color(&state, 0, 1, 254, 255));
    assert(mv_draw_state_background(&state, 255, 254, 0));
    memcpy(&before, &state, sizeof(state));
    int64_t invalid[] = {-1, 256, INT64_MIN, INT64_MAX};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(*invalid); ++i) {
        int64_t v = invalid[i];
        assert(!mv_draw_state_color(&state, v, 0, 0, 0));
        assert(!mv_draw_state_color(&state, 0, v, 0, 0));
        assert(!mv_draw_state_color(&state, 0, 0, v, 0));
        assert(!mv_draw_state_color(&state, 0, 0, 0, v));
        assert(!mv_draw_state_background(&state, v, 0, 0));
        assert(!mv_draw_state_background(&state, 0, v, 0));
        assert(!mv_draw_state_background(&state, 0, 0, v));
        assert(memcmp(&before, &state, sizeof(state)) == 0);
    }
    assert(!mv_draw_state_color(NULL, 0, 0, 0, 0));
    assert(!mv_draw_state_background(NULL, 0, 0, 0));
    MvDrawState copy = state;
    assert(mv_draw_state_init(&state));
    assert(copy.color.g == 1 && copy.background.r == 255);
    return 0;
}
