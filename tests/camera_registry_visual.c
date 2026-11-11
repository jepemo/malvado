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
    static MvCameraRef explicit;
    if (*stage == 5) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef actor;
    assert(mv_process_ref_create(ctx, 1, &actor) == MV_OK);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color a = pixel(image, 20, 20), b = pixel(image, 10, 10);
    Color c = pixel(image, 40, 40), edge = pixel(image, 39, 40);
    UnloadImage(image);
    MvCamera value;
    if (*stage == 0) {
        assert(a.r > 240 && b.r < 10);
        MvCameraRef main;
        assert(mv_camera_main(ctx, &main) == MV_OK);
        assert(mv_camera_init(&value, 10, 10, NULL));
        assert(mv_camera_put(ctx, main, value) == MV_OK);
        assert(mv_process_signal(ctx, actor.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else if (*stage == 1) {
        assert(b.r > 240 && a.r < 10);
        MvCameraViewport viewport = {40, 40, 10, 10};
        assert(mv_camera_init(&value, 0, 0, &viewport));
        assert(mv_camera_create(ctx, value, &explicit) == MV_OK);
        assert(mv_process_camera_set(ctx, actor, &explicit) == MV_OK);
        assert(mv_process_set_position(ctx, actor, 0, 0) == MV_OK);
    } else if (*stage == 2) {
        assert(c.r > 240 && edge.r < 10);
        assert(mv_camera_close(ctx, explicit) == MV_OK);
    } else if (*stage == 3) {
        assert(c.r < 10 && a.r < 10 && b.r < 10);
        MvCameraRef screen = {0};
        assert(mv_process_camera_set(ctx, actor, &screen) == MV_OK);
        assert(mv_process_set_position(ctx, actor, 20, 20) == MV_OK);
    } else {
        assert(a.r > 240 && b.r < 10);
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
    assert(stage == 5);
    return 0;
}
