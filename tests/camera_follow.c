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
#include "malvado/camera.h"
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

static MvStepResult step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return user ? MV_STEP_DONE : MV_STEP_YIELD;
}

static void destroyed(void *user)
{
    ++*(int *)user;
}

static void trace(const char *phase, MvCamera camera)
{
    printf("%s x=%.0f y=%.0f\n", phase, camera.x, camera.y);
}

int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvProcessId id;
    MvProcessRef target;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &target) == MV_OK);
    MvCamera camera;
    assert(mv_camera_init(&camera, 3, 4, NULL));
    assert(mv_process_set_position(ctx, target, 100, 80) == MV_OK);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    assert(camera.x == 50 && camera.y == 40);
    trace("follow", camera);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_set_position(ctx, target, 120, 90) == MV_OK);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    assert(camera.x == 70 && camera.y == 50);
    trace("frozen", camera);
    MvCameraViewport viewport = {10, 20, 40, 60};
    assert(mv_camera_init(&camera, 0, 0, &viewport));
    MvCamera copy = camera;
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    assert(camera.x == 100 && camera.y == 60 && copy.x == 0 && copy.y == 0);
    trace("viewport", camera);
    MvCamera before = camera;
    MvProcessRef saved = target;
    assert(mv_camera_follow_step(&camera, other, &target, 100, 80) == MV_INVALID_ARGUMENT);
    assert(mv_camera_follow_step(NULL, ctx, &target, 100, 80) == MV_INVALID_ARGUMENT);
    assert(mv_camera_follow_step(&camera, NULL, &target, 100, 80) == MV_INVALID_ARGUMENT);
    assert(mv_camera_follow_step(&camera, ctx, NULL, 100, 80) == MV_INVALID_ARGUMENT);
    assert(mv_camera_follow_step(&camera, ctx, &target, 0, 80) == MV_INVALID_ARGUMENT);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 0) == MV_INVALID_ARGUMENT);
    assert(mv_process_set_position(ctx, target, DBL_MAX, 0) == MV_OK);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_INVALID_ARGUMENT);
    assert(memcmp(&before, &camera, sizeof(camera)) == 0);
    assert(memcmp(&saved, &target, sizeof(target)) == 0);
    assert(mv_process_set_position(ctx, target, 120, 90) == MV_OK);
    assert(mv_process_signal(ctx, id, MV_SIGNAL_SLEEP, 0) == MV_OK);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    int killed;
    assert(mv_process_kill(ctx, target, &killed) == MV_OK && killed);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    assert(target.id == 0 && target.runtime.high == 0 && target.runtime.low == 0 && target.runtime.sequence == 0);
    assert(camera.x == 100 && camera.y == 60);
    trace("ended", camera);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    target = saved;
    target.id = 0;
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_INVALID_ARGUMENT);
    MvProcessId owner, child;
    int drops = 0;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, NULL, &owner) == MV_OK);
    assert(mv_process_spawn_related(ctx, owner, owner, 0, step, destroyed, &drops, &child) == MV_OK);
    assert(mv_process_ref_create(ctx, child, &target) == MV_OK);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK);
    before = camera;
    MvProcessRef owner_ref;
    assert(mv_process_ref_create(ctx, owner, &owner_ref) == MV_OK);
    assert(mv_process_kill(ctx, owner_ref, &killed) == MV_OK && killed);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(drops == 1);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK && !target.id);
    assert(memcmp(&before, &camera, sizeof(camera)) == 0);
    drops = 0;
    assert(mv_process_spawn(ctx, 0, 0, step, destroyed, &drops, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &target) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK && drops == 1);
    assert(mv_camera_follow_step(&camera, ctx, &target, 100, 80) == MV_OK && !target.id);
    assert(memcmp(&before, &camera, sizeof(camera)) == 0);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
