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
#include "malvado/platform.h"

static void frame(void *user)
{
    int *stage = user;
    if (*stage == 3) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx, 1, &ref) == MV_OK);
    Image screen = LoadImageFromScreen();
    Color pixel = GetImageColor(screen, 10 * screen.width / GetScreenWidth(),
                                10 * screen.height / GetScreenHeight());
    UnloadImage(screen);
    MvImageGraphic graph;
    int present;
    assert(mv_process_image_get(ctx, ref, &graph, &present) == MV_OK);
    if (*stage == 0) {
        assert(present && ((graph.region.width == 4 && graph.region.height == 8) ||
                            (graph.region.width == 2 && graph.region.height == 2)));
        assert(pixel.r > 240 && pixel.g < 10);
        const uint8_t pixels[] = {0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255};
        MvImage *image = mv_image_from_rgba(2,2,pixels,sizeof(pixels));
        assert(image);
        assert(mv_process_image_set(ctx, ref, (MvImageGraphic){image,{0,0,2,2},0,0}) == MV_OK);
        mv_image_destroy(image);
        assert(mv_process_signal(ctx, ref.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else if (*stage == 1) {
        assert(present && graph.region.width == 2 && pixel.g > 240 && pixel.r < 10);
        assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
    } else {
        assert(!present && pixel.r < 10 && pixel.g < 10);
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
