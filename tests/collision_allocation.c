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
#include "malvado/collision.h"

static int fail_allocation;

void *test_collision_calloc(size_t count, size_t size)
{
    return fail_allocation ? NULL : calloc(count, size);
}

void *test_collision_realloc(void *ptr, size_t size)
{
    return fail_allocation ? NULL : realloc(ptr, size);
}

int
main(void)
{
    fail_allocation = 1;
    assert(!mv_collision_world_create());
    fail_allocation = 0;
    MvCollisionWorld *world = mv_collision_world_create();
    assert(world);
    size_t id = 99;
    fail_allocation = 1;
    assert(!mv_collision_world_add_rect(world, (MvRect){0, 0, 4, 4}, &id));
    assert(id == 99);
    fail_allocation = 0;
    for (int i = 0; i < 16; ++i)
        assert(mv_collision_world_add_rect(world, (MvRect){i * 10, 0, 4, 4}, &id));
    fail_allocation = 1;
    id = 99;
    assert(!mv_collision_world_add_rect(world, (MvRect){-10, 0, 4, 4}, &id));
    assert(id == 99);
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    assert(mv_collision_world_query(world, shape, (MvCollisionTransform){1, 1, 0, 1}, &id) == 1);
    assert(id == 1);
    fail_allocation = 0;
    assert(mv_collision_world_add_rect(world, (MvRect){-10, 0, 4, 4}, &id));
    assert(id == 17);
    mv_collision_world_destroy(world);
    return 0;
}
