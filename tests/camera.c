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
#include <float.h>
#include <math.h>
#include <string.h>
#include "malvado/camera.h"

int main(void)
{
    MvCamera camera;
    double x = 7, y = 9;
    assert(mv_camera_init(&camera, 0, 0, NULL));
    assert(mv_camera_project(&camera, 100, 50, 0, 0, &x, &y) && x == 100 && y == 50);
    assert(mv_camera_set(&camera, 100, 50));
    assert(mv_camera_move(&camera, -10, 5) && camera.x == 90 && camera.y == 55);
    assert(mv_camera_project(&camera, 100, 70, 3, 4, &x, &y) && x == 7 && y == 11);
    MvCamera old;
    memcpy(&old, &camera, sizeof(old));
    assert(!mv_camera_set(&camera, NAN, 0));
    assert(!mv_camera_move(&camera, INFINITY, 0));
    assert(!mv_camera_init(&camera, DBL_MAX, 0, NULL));
    MvCameraViewport invalid = {0};
    assert(!mv_camera_init(&camera, 0, 0, &invalid));
    assert(!memcmp(&old, &camera, sizeof(old)));
    assert(!mv_camera_set(NULL, 0, 0));
    assert(!mv_camera_project(NULL, 0, 0, 0, 0, &x, &y));
    assert(!mv_camera_project(&camera, 0, 0, 0, 0, &x, &x));
    double saved_x = x, saved_y = y;
    assert(!mv_camera_project(&camera, NAN, 0, 0, 0, &x, &y));
    assert(x == saved_x && y == saved_y);
    MvCameraViewport viewport = {40, 60, 300, 240};
    assert(mv_camera_init(&camera, 100, 50, &viewport));
    assert(mv_camera_project(&camera, 100, 50, 0, 0, &x, &y) && x == 40 && y == 60);
    assert(mv_camera_project(&camera, 130, 80, 10, 5, &x, &y) && x == 60 && y == 85);
    assert(mv_camera_set(&camera, FLT_MAX / 64, 0));
    assert(!mv_camera_move(&camera, 1e32, 0));
    assert(mv_camera_init(&camera, 0, 0, NULL));
    assert(mv_camera_center(&camera, 100, 80, 100, 80));
    assert(camera.x == 50 && camera.y == 40);
    assert(mv_camera_init(&camera, 0, 0, &viewport));
    assert(mv_camera_center(&camera, 200, 150, 800, 600));
    assert(camera.x == 50 && camera.y == 30);
    assert(!mv_camera_center(&camera, NAN, 0, 800, 600));
    assert(!mv_camera_center(&camera, 200, 150, 0, 600));
    assert(camera.x == 50 && camera.y == 30);
    return 0;
}
