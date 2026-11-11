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
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx, 1, &ref) == MV_OK);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color old = pixel(image, 12, 12), moved = pixel(image, 44, 44);
    Color extended = pixel(image, 44, 50), outside = pixel(image, 50, 44);
    UnloadImage(image);
    if (*stage == 0) {
        assert(old.r > 240 && old.g < 10 && moved.r < 10);
        assert(mv_process_set_position(ctx, ref, 44, 44) == MV_OK);
        assert(mv_process_set_angle(ctx, ref, 90) == MV_OK);
        assert(mv_process_set_scale(ctx, ref, 2) == MV_OK);
        assert(mv_process_signal(ctx, ref.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else if (*stage == 1) {
        assert(old.r < 10 && moved.r > 240 && moved.g < 10);
        assert(extended.r > 240 && outside.r < 10);
        assert(mv_process_set_scale(ctx, ref, 0) == MV_OK);
    } else if (*stage == 2) {
        assert(moved.r < 10 && extended.r < 10);
        assert(mv_process_set_scale(ctx, ref, 2) == MV_OK);
    } else {
        assert(moved.r > 240 && extended.r > 240);
        int killed;
        assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
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
