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

static MvProcessRef target;
static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

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
                            (graph.region.width == 2 && graph.region.height == 2) ||
                            (graph.region.width == 3 && graph.region.height == 5)));
        assert(pixel.r > 240 && pixel.g < 10);
        if (graph.region.width == 3)
            assert(graph.pivot_x == 1.5 && graph.pivot_y == 2.5);
        MvProcessId id;
        assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
        assert(mv_process_ref_create(ctx, id, &target) == MV_OK);
        MvGraphic point = {0};
        assert(mv_graphic_circle(0.25, &point.shape));
        assert(mv_process_graphic_set(ctx, target, point) == MV_OK);
        assert(mv_process_set_position(ctx, target, 13, 11) == MV_OK);
        int overlap;
        assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK);
        assert(overlap == (graph.region.width == 4));
        const uint8_t pixels[] = {0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255};
        MvImage *image = mv_image_from_rgba(2,2,pixels,sizeof(pixels));
        assert(image);
        assert(mv_process_image_set(ctx, ref, (MvImageGraphic){image,{0,0,2,2},0,0}) == MV_OK);
        mv_image_destroy(image);
        /* Query C immediately: no Lua refresh, tick or render may repair it. */
        assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && !overlap);
        assert(mv_process_set_position(ctx, ref, 12, 10) == MV_OK);
        assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && overlap);
        assert(mv_process_set_position(ctx, ref, 10, 10) == MV_OK);
        assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && !overlap);
        assert(mv_process_signal(ctx, ref.id, MV_SIGNAL_FREEZE, 0) == MV_OK);
    } else if (*stage == 1) {
        assert(present && graph.region.width == 2 && pixel.g > 240 && pixel.r < 10);
        assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
        int overlap = 9;
        assert(mv_process_collider_test(ctx, ref, target, &overlap) == MV_OK && !overlap);
    } else {
        assert(!present && pixel.r < 10 && pixel.g < 10);
        int killed;
        assert(mv_process_kill(ctx, ref, &killed) == MV_OK && killed);
        assert(mv_process_kill(ctx, target, &killed) == MV_OK && killed);
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
