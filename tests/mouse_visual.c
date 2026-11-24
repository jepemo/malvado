/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
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
    unsigned pressed = 0, released = 0;
    for (int frame = 0; frame < 600 && !mv_window_should_close(); ++frame) {
        mv_input_tick();
        for (int i = MV_MOUSE_LEFT; i <= MV_MOUSE_RIGHT; ++i) {
            MvMouseButton button = (MvMouseButton)i;
            if (mv_mouse_pressed(button) && mv_mouse_pressed(button))
                pressed |= 1u << (i - MV_MOUSE_LEFT);
            if (mv_mouse_released(button) && mv_mouse_released(button))
                released |= 1u << (i - MV_MOUSE_LEFT);
        }
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
    return phase == 2 && pressed == 7 && released == 7 ? 0 : 1;
}
