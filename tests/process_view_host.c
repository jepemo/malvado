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

static void between_ticks(void *user)
{
    int *stage = user;
    if (*stage == 2) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    if (mv_process_ref_create(ctx,1,&ref) != MV_OK) return;
    float x,y;
    assert(mv_process_scroll_get(ctx,ref,&x,&y) == MV_OK);
    if (*stage == 0) {
        assert(x == 1 && y == 2);
        MvCameraViewport region = {10,20,30,40};
        assert(mv_process_region_set(ctx,ref,&region) == MV_OK);
        assert(mv_process_scroll_set(ctx,ref,3,4) == MV_OK);
        ++*stage;
    } else if (x == 5 && y == 6) {
        MvCameraViewport region;
        int present;
        assert(mv_process_region_get(ctx,ref,&region,&present) == MV_OK && present && region.x == 7 && region.width == 9);
        assert(mv_process_region_set(ctx,ref,NULL) == MV_OK);
        assert(mv_process_scroll_set(ctx,ref,0,0) == MV_OK);
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
