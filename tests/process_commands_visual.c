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

typedef struct { int stage; int64_t appended_tick; } Probe;

static Color pixel(int x, int y)
{
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    Color result = GetImageColor(screen,x*screen.width/GetScreenWidth(),y*screen.height/GetScreenHeight());
    UnloadImage(screen);
    return result;
}

static void check_frame(void *user)
{
    Probe *probe = user;
    if (probe->stage == 3) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx,1,&ref) == MV_OK);
    MalvadoProcessInfo info;
    size_t total, count;
    int64_t tick;
    assert(malvado_debug_snapshot(&info,1,&total,&tick) && total == 1);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK);
    if (tick == 0) return;
    Color overlap = pixel(13,16), outside = pixel(28,28);
    if (probe->stage == 0) {
        assert(count == 5 && overlap.r == 255 && overlap.g == 0 && outside.g == 255);
        /* A slow display runner can skip several ticks between frames. Append
         * early in the 12-tick cadence period so the duration check below tests
         * persistence, independently of when the first frame was presented. */
        if (tick < 3 || (tick - 1) % 12 > 3) return;
        assert(mv_process_draw_box(ctx,ref,24,24,2,2,1,(MvColor){255,255,0,255}) == MV_OK);
        probe->appended_tick = tick;
        probe->stage = 1;
    } else if (probe->stage == 1) {
        if (count == 6) {
            Color added = pixel(24,24);
            assert(added.r == 255 && added.g == 255);
            return;
        }
        assert(count == 5 && tick - probe->appended_tick >= 8);
        Color reset = pixel(24,24);
        assert(reset.r == 0 && reset.g == 255);
        assert(mv_process_signal(ctx,ref.id,MV_SIGNAL_FREEZE,0) == MV_OK);
        assert(mv_process_draw_clear(ctx,ref) == MV_OK);
        probe->stage = 2;
    } else {
        assert(count == 0 && outside.b == 255 && outside.g == 0 && overlap.b == 255);
        int killed;
        assert(mv_process_kill(ctx,ref,&killed) == MV_OK && killed);
        probe->stage = 3;
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    Probe probe = {0};
    char *args[] = {"malvado-run",argv[1]};
    malvado_debug_set_frame_hook(check_frame,&probe);
    assert(malvado_run(2,args) == 0);
    malvado_debug_set_frame_hook(NULL,NULL);
    assert(probe.stage == 3);
    return 0;
}
