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
    if (!mv_window_open(160, 120, "Malvado C mouse"))
        return 1;
    int phase = 0;
    for (int frame = 0; frame < 600 && !mv_window_should_close(); ++frame) {
        int x, y;
        int position = mv_mouse_position(&x, &y);
        int held = mv_mouse_down(MV_MOUSE_LEFT) + mv_mouse_down(MV_MOUSE_MIDDLE) +
                   mv_mouse_down(MV_MOUSE_RIGHT);
        if (phase == 0 && position && x == 50 && y == 45 && held == 3)
            phase = 1;
        else if (phase == 1 && held == 0) {
            phase = 2;
            break;
        }
        mv_window_begin((MvColor){0, 0, 0, 255});
        mv_window_end();
    }
    mv_window_close();
    return phase == 2 ? 0 : 1;
}
