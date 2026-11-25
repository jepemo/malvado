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

int
main(void)
{
    if (!mv_window_open(160, 120, "Malvado Image"))
        return 1;

    MvContext *game = mv_context_create(60, 5);
    MvImage *image = mv_image_load_for_game(game, "tile.png");
    if (!image) {
        mv_context_destroy(game);
        mv_window_close();
        return 1;
    }
    printf("image: %dx%d\n", mv_image_width(image), mv_image_height(image));
    for (int frame = 0; frame < 120 && !mv_window_should_close(); ++frame) {
        mv_window_begin((MvColor){0, 0, 0, 255});
        mv_image_draw(image, 32, 32);
        mv_window_end();
    }

    /* A direct image reference belongs to the caller and needs a live window. */
    mv_image_destroy(image);
    mv_context_destroy(game);
    mv_window_close();
    return 0;
}
