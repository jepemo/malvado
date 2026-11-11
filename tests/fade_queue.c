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

int
main(void)
{
    MvFadeQueue queue = {0}, before;
    MvColor white = {255, 255, 255, 255};
    assert(!mv_fade_queue_push(NULL, white, 1));
    assert(!mv_fade_queue_step(NULL));
    assert(!mv_fade_queue_active(NULL));
    assert(!mv_fade_queue_active(&queue));
    assert(!mv_fade_queue_step(&queue));
    memcpy(&before, &queue, sizeof(queue));
    assert(!mv_fade_queue_push(&queue, white, 1000001));
    assert(memcmp(&before, &queue, sizeof(queue)) == 0);
    assert(mv_fade_queue_push(&queue, white, 1000000) == 1);
    assert(mv_fade_queue_step(&queue));
    assert(queue.fade.current.a == 0);
    queue = (MvFadeQueue){0};
    /* Wrap the FIFO repeatedly and prove that zero-tick work drains in order. */
    for (unsigned round = 0; round < 40; ++round) {
        assert(mv_fade_queue_push(&queue, white, 1) == 1);
        for (unsigned i = 0; i < 16; ++i)
            assert(mv_fade_queue_push(&queue, (MvColor){i, i + 1, i + 2, i + 3}, 0) == 1);
        memcpy(&before, &queue, sizeof(queue));
        assert(mv_fade_queue_push(&queue, white, 0) == -1);
        assert(memcmp(&before, &queue, sizeof(queue)) == 0);
        assert(!mv_fade_queue_push(&queue, white, 1000001));
        assert(memcmp(&before, &queue, sizeof(queue)) == 0);
        assert(mv_fade_queue_step(&queue));
        assert(!mv_fade_queue_active(&queue));
        assert(queue.fade.current.r == 15 && queue.fade.current.a == 18);
        assert(!mv_fade_queue_step(&queue));
    }
    assert(mv_fade_queue_push(&queue, (MvColor){1, 2, 3, 4}, 0) == 1);
    assert(!mv_fade_queue_active(&queue));
    assert(queue.fade.current.a == 4);
    assert(mv_fade_queue_push(&queue, (MvColor){0, 0, 0, 0}, 2) == 1);
    assert(mv_fade_queue_step(&queue) && queue.fade.current.a == 2);
    assert(mv_fade_queue_step(&queue) && queue.fade.current.a == 0);
    return 0;
}
