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
#include "runner/runtime.h"
#include "malvado/camera.h"

typedef struct { int stage; MvCameraRef camera; } State;
static void between_ticks(void *user)
{
    State *state = user;
    if (state->stage == 4) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef actor;
    if (mv_process_ref_create(ctx, 1, &actor) != MV_OK) return;
    MvCameraRef selected;
    MvCamera value;
    assert(mv_process_camera_get(ctx, actor, &selected) == MV_OK);
    if (state->stage == 0) {
        assert(mv_camera_get(ctx, selected, &value) == MV_OK && value.x == 5 && value.y == 6);
        value.x = 20; value.y = 30;
        assert(mv_camera_put(ctx, selected, value) == MV_OK);
        ++state->stage;
    } else if (state->stage == 1) {
        assert(mv_camera_get(ctx, selected, &value) == MV_OK);
        if (value.x != 21 || value.y != 32) return;
        assert(mv_camera_init(&value, 40, 50, NULL));
        assert(mv_camera_create(ctx, value, &state->camera) == MV_OK);
        assert(mv_process_camera_set(ctx, actor, &state->camera) == MV_OK);
        ++state->stage;
    } else if (state->stage == 2 && !selected.id) {
        assert(mv_camera_get(ctx, state->camera, &value) == MV_OK && value.x == 41 && value.y == 52);
        assert(mv_process_camera_set(ctx, actor, &state->camera) == MV_OK);
        ++state->stage;
    } else if (state->stage == 3) {
        double x, y;
        assert(mv_process_position(ctx, actor, &x, &y) == MV_OK);
        if (x != 2) return;
        assert(mv_camera_close(ctx, state->camera) == MV_OK);
        assert(mv_process_set_position(ctx, actor, x, 1) == MV_OK);
        ++state->stage;
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    State state = {0};
    char *args[] = {"malvado-run", "--headless", argv[1]};
    malvado_debug_set_hook(between_ticks, &state);
    assert(malvado_run(3, args) == 0);
    malvado_debug_set_hook(NULL, NULL);
    assert(state.stage == 4);
    return 0;
}
