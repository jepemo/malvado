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
#include "raylib.h"
#include "runner/runtime.h"
#include "malvado/camera.h"

static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void frame(void *user)
{
    int *stage = user;
    if (*stage == 4) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef actor;
    assert(mv_process_ref_create(ctx, 1, &actor) == MV_OK);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color old = pixel(image, 20, 20), moved = pixel(image, 15, 20);
    Color edge = pixel(image, 17, 20);
    UnloadImage(image);
    if (*stage == 0) {
        assert(old.r > 240 && moved.r < 10);
        assert(mv_process_signal(ctx, actor.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
        assert(mv_process_scroll_set(ctx, actor, 5, 0) == MV_OK);
        MvCameraViewport region = {15, 20, 2, 2};
        assert(mv_process_region_set(ctx, actor, &region) == MV_OK);
    } else if (*stage == 1) {
        assert(old.r < 10 && moved.r > 240 && edge.r < 10);
        MvCameraViewport offscreen = {100, 100, 2, 2};
        assert(mv_process_region_set(ctx, actor, &offscreen) == MV_OK);
    } else if (*stage == 2) {
        assert(old.r < 10 && moved.r < 10 && edge.r < 10);
        assert(mv_process_region_set(ctx, actor, NULL) == MV_OK);
        assert(mv_process_scroll_set(ctx, actor, 0, 0) == MV_OK);
    } else {
        assert(old.r > 240 && moved.r < 10);
        double x, y;
        assert(mv_process_position(ctx, actor, &x, &y) == MV_OK);
        assert(x == 20 && y == 20);
        int killed;
        assert(mv_process_kill(ctx, actor, &killed) == MV_OK && killed);
    }
    ++*stage;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    int stage = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(frame, &stage);
    assert(malvado_run(2, args) == 0);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(stage == 4);
    return 0;
}
