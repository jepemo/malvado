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
#include <math.h>
#include <string.h>
static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvCameraRef main, second, selection;
    assert(mv_camera_main(ctx, &main) == MV_OK && main.id == 1);
    MvCamera value, out;
    assert(mv_camera_get(ctx, main, &value) == MV_OK && value.x == 0 && value.y == 0);
    assert(mv_camera_init(&value, 10, 20, NULL));
    assert(mv_camera_create(ctx, value, &second) == MV_OK && second.id == 2);
    assert(mv_camera_get(ctx, second, &out) == MV_OK && out.x == 10);
    value.x = 30;
    assert(mv_camera_get(ctx, second, &out) == MV_OK && out.x == 10);
    assert(mv_camera_put(ctx, main, value) == MV_OK);
    MvProcessId id;
    MvProcessRef process;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &process) == MV_OK);
    assert(mv_process_camera_get(ctx, process, &selection) == MV_OK && selection.id == 1);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_OK && out.x == 30);
    assert(mv_process_camera_set(ctx, process, &second) == MV_OK);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_OK && out.x == 10);
    MvCamera before = out;
    MvCameraRef saved = selection;
    value.x = NAN;
    assert(mv_camera_put(ctx, second, value) == MV_INVALID_ARGUMENT);
    assert(mv_camera_create(ctx, value, &selection) == MV_INVALID_ARGUMENT);
    assert(memcmp(&saved, &selection, sizeof(saved)) == 0);
    assert(mv_camera_get(other, second, &out) == MV_INVALID_ARGUMENT);
    assert(mv_camera_put(other, second, before) == MV_INVALID_ARGUMENT);
    assert(mv_camera_close(other, second) == MV_INVALID_ARGUMENT);
    assert(mv_camera_get(ctx, second, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_camera_main(NULL, &selection) == MV_INVALID_ARGUMENT);
    assert(mv_camera_main(ctx, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_camera_create(ctx, before, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_camera_get(ctx, process, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_camera_resolve(ctx, process, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_camera_get(other, process, &selection) == MV_INVALID_ARGUMENT);
    MvCameraRef foreign;
    assert(mv_camera_main(other, &foreign) == MV_OK);
    assert(mv_process_camera_set(ctx, process, &foreign) == MV_INVALID_ARGUMENT);
    assert(memcmp(&before, &out, sizeof(out)) == 0);
    assert(mv_camera_close(ctx, main) == MV_INVALID_ARGUMENT);
    assert(mv_camera_close(ctx, second) == MV_OK);
    assert(mv_camera_close(ctx, second) == MV_OK);
    assert(mv_camera_get(ctx, second, &out) == MV_NOT_FOUND);
    assert(mv_process_camera_get(ctx, process, &selection) == MV_OK && selection.id == second.id);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_NOT_FOUND);
    assert(mv_process_camera_set(ctx, process, &second) == MV_NOT_FOUND);
    assert(mv_process_camera_get(ctx, process, &selection) == MV_OK && selection.id == second.id);
    MvCameraRef screen = {0};
    assert(mv_process_camera_set(ctx, process, &screen) == MV_OK);
    assert(mv_process_camera_get(ctx, process, &selection) == MV_OK && !selection.id);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_OK && out.x == 0);
    assert(mv_process_camera_set(ctx, process, NULL) == MV_OK);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_OK && out.x == 30);
    value = before;
    for (unsigned i = 0; i < 500; ++i) {
        MvCameraRef next;
        assert(mv_camera_create(ctx, value, &next) == MV_OK && next.id > second.id);
        if (i % 2 == 0) assert(mv_camera_close(ctx, next) == MV_OK);
        second = next;
    }
    int killed;
    assert(mv_process_kill(ctx, process, &killed) == MV_OK && killed);
    assert(mv_process_camera_set(ctx, process, NULL) == MV_NOT_FOUND);
    assert(mv_process_camera_get(ctx, process, &selection) == MV_NOT_FOUND);
    assert(mv_process_camera_resolve(ctx, process, &out) == MV_NOT_FOUND);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
