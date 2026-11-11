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
static void frame(void *user)
{
    int *stage = user;
    if (*stage == 4) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef red, green;
    assert(mv_process_ref_create(ctx, 1, &red) == MV_OK);
    assert(mv_process_ref_create(ctx, 2, &green) == MV_OK);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color pixel = GetImageColor(image, 20 * image.width / GetScreenWidth(),
                                20 * image.height / GetScreenHeight());
    UnloadImage(image);
    if (*stage == 1) assert(pixel.g > 240 && pixel.r < 10);
    else assert(pixel.r > 240 && pixel.g < 10);
    MvProcessDrawInfo order[2];
    size_t count;
    assert(mv_process_draw_order(ctx, order, 2, &count) == MV_OK);
    double z;
    assert(mv_process_z(ctx, red, &z) == MV_OK);
    if (*stage == 0) {
        assert(z == 10 && count == 2 && order[0].id == 2 && order[1].id == 1);
        assert(mv_process_set_z(ctx, red, -1) == MV_OK);
    } else if (*stage == 1) {
        /* Wait until a Lua activation reads the native mutation and writes back. */
        if (z != -2) return;
        assert(count == 2 && order[0].id == 1 && order[1].id == 2);
        assert(mv_process_signal(ctx, 2, MV_SIGNAL_SLEEP, 0) == MV_OK);
    } else if (*stage == 2) {
        assert(count == 1 && order[0].id == 1);
        assert(mv_process_signal(ctx, 1, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else {
        assert(count == 1 && order[0].id == 1);
        int killed;
        assert(mv_process_kill(ctx, red, &killed) == MV_OK && killed);
        assert(mv_process_kill(ctx, green, &killed) == MV_OK && killed);
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
