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

int
main(void)
{
    const MvKey keys[] = {MV_KEY_LEFT, MV_KEY_RIGHT, MV_KEY_UP, MV_KEY_DOWN,
                          MV_KEY_A, MV_KEY_0, MV_KEY_SPACE};
    if (!mv_window_open(160, 120, "Malvado C keys"))
        return 1;
    int phase = 0;
    unsigned pressed = 0, released = 0;
    for (int frame = 0; frame < 600 && !mv_window_should_close(); ++frame) {
        mv_input_tick();
        int held = 0;
        for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
            held += mv_key_down(keys[i]);
            int edge = mv_key_pressed(keys[i]);
            if (edge != mv_key_pressed(keys[i])) return 1;
            pressed += edge;
            released += mv_key_released(keys[i]);
        }
        if (phase == 0 && held == (int)(sizeof(keys) / sizeof(keys[0])))
            phase = 1;
        else if (phase == 1 && held == 0)
            phase = 2;
        else if (phase == 2 && mv_key_pressed(MV_KEY_ESCAPE)) {
            phase = 3;
            break;
        }
        mv_input_tick(); /* an extra catch-up tick has no new edge */
        for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
            if (mv_key_pressed(keys[i]) || mv_key_released(keys[i])) return 1;
        mv_window_begin((MvColor){0, 0, 0, 255});
        mv_window_end();
    }
    mv_window_close();
    return phase == 3 && pressed == 7 && released == 7 ? 0 : 1;
}
