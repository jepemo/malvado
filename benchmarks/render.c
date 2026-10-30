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

#include <stdio.h>
#include <string.h>
#include "malvado/platform.h"

int
main(int argc, char **argv)
{
    int smoke = argc == 2 && strcmp(argv[1], "--smoke") == 0;
    const int target_frames = smoke ? 3 : 120;
    const int sprites = 1000;
    int frames = 0;
    int ok = 1;
    double draw_seconds = 0, present_seconds = 0;
    if (!mv_window_open(800, 600, "Render benchmark"))
        return 1;
    MvImage *image = mv_image_load("examples/bench/assets/colours/000.png");
    if (!image) {
        mv_window_close();
        return 1;
    }
    while (frames < target_frames && !mv_window_should_close()) {
        double before, drawn, presented;
        if (!mv_monotonic_time(&before)) {
            ok = 0;
            break;
        }
        mv_window_begin((MvColor){0, 0, 0, 255});
        for (int i = 0; i < sprites; ++i)
            mv_image_draw(image, (i * 31) % 780, (i * 47) % 580);
        if (!mv_monotonic_time(&drawn)) {
            mv_window_end();
            ok = 0;
            break;
        }
        mv_window_end();
        if (!mv_monotonic_time(&presented)) {
            ok = 0;
            break;
        }
        draw_seconds += drawn - before;
        present_seconds += presented - drawn;
        ++frames;
    }
    if (frames > 0)
        printf("render frames=%d sprites=%d draw_ms=%.3f present_wait_ms=%.3f\n",
               frames, sprites, 1000 * draw_seconds / frames, 1000 * present_seconds / frames);
    mv_image_destroy(image);
    mv_window_close();
    return ok && frames == target_frames ? 0 : 1;
}
