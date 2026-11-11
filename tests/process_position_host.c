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

static void between_ticks(void *user)
{
    int *stage = user;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    if (mv_process_ref_create(ctx, 1, &ref) != MV_OK) return;
    double x, y;
    if (mv_process_position(ctx, ref, &x, &y) != MV_OK) return;
    if (*stage == 0 && x == 4 && y == 6) {
        assert(mv_process_set_position(ctx, ref, 12, 8) == MV_OK);
        assert(mv_process_set_angle(ctx, ref, 90) == MV_OK);
        assert(mv_process_set_scale(ctx, ref, 2) == MV_OK);
        ++*stage;
    } else if (*stage == 1 && x == 19.5 && y == 8) {
        double angle, scale;
        assert(mv_process_angle(ctx, ref, &angle) == MV_OK && angle == 180);
        assert(mv_process_scale(ctx, ref, &scale) == MV_OK && scale == .5);
        assert(mv_process_set_position(ctx, ref, -2, 5) == MV_OK);
        assert(mv_process_set_angle(ctx, ref, 270) == MV_OK);
        assert(mv_process_set_scale(ctx, ref, 3) == MV_OK);
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
