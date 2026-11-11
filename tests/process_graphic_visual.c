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
#include "malvado/graphic.h"

static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void frame(void *user)
{
    int *stage = user;
    if (*stage == 3) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx, 1, &ref) == MV_OK);
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color center = pixel(image, 12, 12), edge = pixel(image, 18, 12);
    UnloadImage(image);
    if (*stage == 0) {
        assert(center.r > 240 && center.g < 10 && edge.r < 10);
        MvGraphic graphic = {.color = {0, 255, 0, 255}};
        assert(mv_graphic_circle(8, &graphic.shape));
        assert(mv_process_graphic_set(ctx, ref, graphic) == MV_OK);
        assert(mv_process_signal(ctx, ref.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else if (*stage == 1) {
        assert(center.g > 240 && center.r < 10 && edge.g > 240);
        assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    } else {
        assert(center.g < 10 && center.r < 10 && edge.g < 10);
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
    assert(stage == 3);
    return 0;
}
