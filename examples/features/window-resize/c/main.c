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

#include <stdio.h>
#include "malvado/platform.h"

static void
show_window(void)
{
    for (int frame = 0; frame < 60 && !mv_window_should_close(); ++frame) {
        mv_window_begin((MvColor){20, 30, 40, 255});
        mv_window_end();
    }
}

static void
print_size(void)
{
    int width, height;
    if (mv_window_size(&width, &height))
        printf("window: %dx%d\n", width, height);
}

int
main(void)
{
    if (!mv_window_open(320, 240, "Malvado Window Resize"))
        return 1;

    print_size();
    show_window();
    if (!mv_window_should_close()) {
        /* Request the resize between draw frames. */
        if (!mv_window_set_size(640, 360)) {
            mv_window_close();
            return 1;
        }
        show_window();
        print_size();
    }
    mv_window_close();
    return 0;
}
