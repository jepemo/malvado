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
#include "malvado/graphic.h"

static void between_ticks(void *user)
{
    int *stage = user;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    if (mv_process_ref_create(ctx, 1, &ref) != MV_OK) return;
    MvGraphic graphic;
    int present;
    if (mv_process_graphic_get(ctx, ref, &graphic, &present) != MV_OK) return;
    if (*stage == 0 && present && graphic.shape.width == 2) {
        assert(mv_graphic_circle(5, &graphic.shape));
        graphic.color = (MvColor){255, 0, 0, 255};
        assert(mv_process_graphic_set(ctx, ref, graphic) == MV_OK);
        MvProcessRef sensor;
        int overlap;
        assert(mv_process_ref_create(ctx, 2, &sensor) == MV_OK);
        assert(mv_process_collider_test(ctx, ref, sensor, &overlap) == MV_OK && overlap);
        ++*stage;
    } else if (*stage == 1 && present && graphic.shape.width == 4) {
        assert(graphic.shape.offset_x == 2 && graphic.color.g == 255);
        assert(mv_process_graphic_clear(ctx, ref) == MV_OK);
        MvProcessRef sensor;
        int overlap;
        assert(mv_process_ref_create(ctx, 2, &sensor) == MV_OK);
        assert(mv_process_collider_test(ctx, ref, sensor, &overlap) == MV_OK && !overlap);
        ++*stage;
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    int stage = 0;
    char *args[] = {"malvado-run", "--headless", argv[1]};
    malvado_debug_set_hook(between_ticks, &stage);
    assert(malvado_run(3, args) == 0);
    malvado_debug_set_hook(NULL, NULL);
    assert(stage == 2);
    return 0;
}
