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
#include <stdio.h>
#include "malvado/platform.h"

int
main(void)
{
    MvFadeQueue queue = {0};
    assert(mv_fade_queue_push(&queue, (MvColor){0, 0, 0, 255}, 2) == 1);
    assert(mv_fade_queue_push(&queue, (MvColor){20, 40, 60, 80}, 0) == 1);
    assert(mv_fade_queue_push(&queue, (MvColor){0, 20, 40, 0}, 2) == 1);
    assert(mv_fade_queue_push(&queue, (MvColor){1, 2, 3, 4}, 0) == 1);
    for (unsigned tick = 0; tick < 5; ++tick) {
        MvColor color = queue.fade.current;
        printf("%u %d %d %d %d %d\n", tick, color.r, color.g, color.b, color.a,
               mv_fade_queue_active(&queue));
        (void)mv_fade_queue_step(&queue);
    }
    return 0;
}
