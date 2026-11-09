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
#include <stdint.h>
#include "malvado/animation.h"

int main(void)
{
    MvAnimationFrame frames[] = {{7, 100000}, {42, 200000}, {7, 50000}};
    MvAnimation *loop = mv_animation_create(frames, 3, 1);
    MvAnimation *once = mv_animation_create(frames, 3, 0);
    assert(loop && once);
    frames[0].graph_id = 99;
    MvAnimationSample sample = {99, 99};
    assert(mv_animation_sample(loop, 0, &sample) && sample.index == 0 && sample.graph_id == 7);
    assert(mv_animation_sample(loop, 99999, &sample) && sample.index == 0);
    assert(mv_animation_sample(loop, 100000, &sample) && sample.index == 1 && sample.graph_id == 42);
    assert(mv_animation_sample(loop, 350000, &sample) && sample.index == 0);
    assert(mv_animation_sample(once, UINT64_MAX, &sample) && sample.index == 2);
    assert(!mv_animation_sample(NULL, 0, &sample) && sample.index == 2);
    assert(!mv_animation_sample(loop, 0, NULL));
    MvAnimationState a = {0}, b = {0};
    for (int i = 0; i < 6; ++i) assert(mv_animation_advance(loop, &a, 60));
    assert(a.elapsed_us == 100000 && b.elapsed_us == 0);
    assert(!mv_animation_advance(loop, &a, 30) && a.elapsed_us == 100000);
    assert(!mv_animation_advance(loop, &a, 0));
    assert(!mv_animation_advance(loop, &a, 1000001));
    for (int i = 6; i < 21; ++i) assert(mv_animation_advance(loop, &a, 60));
    assert(a.elapsed_us == 0 && a.remainder == 0);
    for (int i = 0; i < 60; ++i) assert(mv_animation_advance(once, &b, 60));
    assert(b.elapsed_us == 350000);
    assert(mv_animation_sample(once, b.elapsed_us, &sample) && sample.index == 2);
    assert(!mv_animation_create(NULL, 1, 1));
    assert(!mv_animation_create(frames, 0, 1));
    assert(!mv_animation_create(frames, 8193, 1));
    assert(!mv_animation_create(frames, 3, 2));
    frames[0].duration_us = 0;
    assert(!mv_animation_create(frames, 3, 1));
    frames[0] = (MvAnimationFrame){-1, 100};
    assert(!mv_animation_create(frames, 3, 1));
    mv_animation_destroy(loop);
    mv_animation_destroy(once);
    mv_animation_destroy(NULL);
    return 0;
}
