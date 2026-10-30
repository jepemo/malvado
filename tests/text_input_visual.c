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
    const char *expected = "aZ9";
    if (!mv_window_open(160, 120, "Malvado C text input"))
        return 1;
    int count = 0;
    int ok = 1;
    for (int frame = 0; frame < 600 && !mv_window_should_close(); ++frame) {
        if (frame >= 60) {
            uint32_t codepoint;
            while (mv_text_input_next(&codepoint)) {
                if (count >= 3 || codepoint != (uint32_t)expected[count])
                    ok = 0;
                ++count;
            }
            if (!ok || count == 3)
                break;
        }
        mv_window_begin((MvColor){0, 0, 0, 255});
        mv_window_end();
    }
    mv_window_close();
    return ok && count == 3 ? 0 : 1;
}
