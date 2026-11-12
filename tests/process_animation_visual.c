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

typedef struct { int stage, frames; } Probe;
static void check_frame(void *user)
{
    Probe *probe = user;
    if (probe->stage == 3) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx,1,&ref) == MV_OK);
    MalvadoProcessInfo info[1];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(info,1,&count,&tick) && count == 1);
    MvImageGraphic image;
    int present;
    assert(mv_process_image_get(ctx,ref,&image,&present) == MV_OK);
    MvAnimationState state;
    MvAnimationSample sample;
    int animated;
    assert(mv_process_animation_clock_get(ctx,ref,&state,&sample,&animated) == MV_OK);
    Image screen = LoadImageFromScreen();
    Color pixel = GetImageColor(screen,10*screen.width/GetScreenWidth(),10*screen.height/GetScreenHeight());
    UnloadImage(screen);
    if (probe->stage == 0) {
        assert(present && animated);
        assert(state.elapsed_us == ((uint64_t)tick * 1000000 / 60) % 100000);
        assert(mv_process_animation_clock_clear(ctx,ref) == MV_BUSY);
        if (sample.graph_id == 7) {
            assert(pixel.r == 252 && pixel.g == 0);
            probe->frames |= 1;
        } else {
            assert(sample.graph_id == 42 && pixel.g == 252 && pixel.r == 0);
            probe->frames |= 2;
        }
        if (probe->frames != 3 || tick < 30) return;
        assert(mv_process_signal(ctx,ref.id,MV_SIGNAL_FREEZE,0) == MV_OK);
        const uint8_t blue[] = {0,0,255,255,0,0,255,255,0,0,255,255,0,0,255,255};
        MvImage *replacement = mv_image_from_rgba(2,2,blue,sizeof(blue));
        assert(replacement);
        assert(mv_process_image_set(ctx,ref,(MvImageGraphic){replacement,{0,0,2,2},0,0}) == MV_OK);
        mv_image_destroy(replacement);
        probe->stage = 1;
    } else if (probe->stage == 1) {
        assert(present && !animated && pixel.b == 255 && pixel.r == 0);
        assert(mv_process_graphic_clear(ctx,ref) == MV_OK);
        probe->stage = 2;
    } else {
        assert(!present && !animated && pixel.r == 0 && pixel.g == 0 && pixel.b == 0);
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
    assert(probe.stage == 3 && probe.frames == 3);
    return 0;
}
