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
#include <stdlib.h>
#include "malvado/animation.h"
static int fail, allocated, released;
void *test_animation_malloc(size_t size)
{
    if (fail) return NULL;
    void *result = malloc(size);
    if (result) ++allocated;
    return result;
}
void test_animation_free(void *pointer)
{
    if (pointer) ++released;
    free(pointer);
}
int main(void)
{
    MvAnimationFrame frame = {7, 100000};
    fail = 1;
    assert(!mv_animation_create(&frame, 1, 1));
    assert(frame.graph_id == 7 && frame.duration_us == 100000 && !allocated);
    fail = 0;
    MvAnimation *animation = mv_animation_create(&frame, 1, 1);
    assert(animation && allocated == 1);
    mv_animation_destroy(animation);
    assert(released == 1);
    return 0;
}
