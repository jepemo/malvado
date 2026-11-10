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
#include "malvado/collision.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

int
main(void)
{
    MvCollisionWorld *world = mv_collision_world_create();
    assert(world);
    size_t id;
    assert(mv_collision_world_add_rect(world, (MvRect){10, 0, 10, 10}, &id) && id == 1);
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionTransform position = {0, 5, 0, 1};
    MvCollisionSweep sweep;
    assert(mv_collision_world_sweep(world, shape, position, 100, 0, &sweep) == 1);
    assert(sweep.hit && sweep.id == 1 && sweep.normal_x == -1 && sweep.normal_y == 0);
    printf("sweep hit=1 fraction=%.6f normal=-1,0\n", sweep.fraction);
    assert(mv_collision_world_move(world, shape, &position, 100, 0, &sweep) == 1);
    assert(mv_collision_world_query(world, shape, position, &id) == 0);
    printf("move x=%.6f y=%.6f clear=1\n", position.x, position.y);
    assert(mv_collision_world_move(world, shape, &position, 0, 1, &sweep) == 1 && !sweep.hit);
    printf("slide x=%.6f y=%.6f\n", position.x, position.y);
    position.x = 11;
    assert(mv_collision_world_move(world, shape, &position, 1, 0, &sweep) == -1);
    assert(position.x == 11 && position.y == 6);
    puts("failure unchanged=1");
    mv_collision_world_destroy(world);
    return 0;
}
