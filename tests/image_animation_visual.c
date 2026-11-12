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
#include <stdio.h>
#include "raylib.h"
#include "malvado/platform.h"
#include "malvado/animation.h"
static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static MvStepResult done(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_DONE;
}
static MvProcessRef spawn(MvContext *ctx)
{
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&id) == MV_OK);
    assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
    return ref;
}
static void released(void *owner) { ++*(int *)owner; }
int main(int argc, char **argv)
{
    assert(argc == 2 && mv_window_open(64,64,"Native image animation"));
    uint8_t red_pixels[16], green_pixels[32];
    for (size_t i=0;i<sizeof(red_pixels);i+=4) {
        red_pixels[i]=255;
        red_pixels[i+1]=0;
        red_pixels[i+2]=0;
        red_pixels[i+3]=255;
    }
    for (size_t i=0;i<sizeof(green_pixels);i+=4) {
        green_pixels[i]=0;
        green_pixels[i+1]=255;
        green_pixels[i+2]=0;
        green_pixels[i+3]=255;
    }
    MvImage *red = mv_image_from_rgba(2,2,red_pixels,sizeof(red_pixels));
    MvImage *green = mv_image_from_rgba(4,2,green_pixels,sizeof(green_pixels));
    assert(red && green);
    MvImageAnimationFrame frames[] = {{{red,{0,0,2,2},0,0},50000},
                                     {{green,{0,0,4,2},0,0},50000}};
    MvContext *ctx = mv_context_create(60,4), *other = mv_context_create(60,4);
    assert(ctx && other);
    MvProcessRef actor=spawn(ctx), target=spawn(ctx);
    MvGraphic dot = {0};
    assert(mv_graphic_circle(0.25,&dot.shape));
    assert(mv_process_graphic_set(ctx,target,dot) == MV_OK);
    assert(mv_process_set_position(ctx,actor,20,20) == MV_OK);
    assert(mv_process_set_position(ctx,target,23,21) == MV_OK);
    int drops=0;
    assert(mv_process_image_animation_set_owned(ctx,actor,frames,0,1,60,&drops,released) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set_owned(other,actor,frames,2,1,60,&drops,released) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set_owned(ctx,actor,frames,2,1,60,&drops,NULL) == MV_INVALID_ARGUMENT);
    assert(drops == 0);
    assert(mv_process_image_animation_set(ctx,actor,NULL,2,1,60) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set(ctx,actor,frames,8193,1,60) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set(ctx,actor,frames,2,2,60) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set(ctx,actor,frames,2,1,0) == MV_INVALID_ARGUMENT);
    assert(mv_process_image_animation_set_owned(ctx,actor,frames,2,1,60,&drops,released) == MV_OK);
    MvImageAnimationFrame invalid_union = frames[0];
    invalid_union.graphic.pivot_x = 1e30;
    invalid_union.graphic.pivot_y = 1e30;
    assert(mv_process_image_animation_set_owned(ctx,actor,&invalid_union,1,1,60,&drops,released) == MV_INVALID_ARGUMENT);
    assert(drops == 0);
    frames[1].duration_us=0;
    assert(mv_process_image_animation_set_owned(ctx,actor,frames,2,1,60,&drops,released) == MV_INVALID_ARGUMENT);
    assert(drops == 0);
    frames[1].duration_us=50000;
    mv_image_destroy(red);
    mv_image_destroy(green);
    MvAnimation *clock = mv_animation_create((MvAnimationFrame[]){{1,1000}},1,1);
    assert(clock);
    assert(mv_process_animation_clock_set(ctx,actor,clock,60) == MV_BUSY);
    assert(mv_process_animation_clock_clear(ctx,actor) == MV_BUSY);
    mv_animation_destroy(clock);
    FILE *file=fopen(argv[1],"r");
    assert(file);
    for (int i=0;i<=6;++i) {
        unsigned long long elapsed;
        int graph;
        assert(fscanf(file,"%llu %d",&elapsed,&graph) == 2);
        MvAnimationState state;
        MvAnimationSample sample;
        int present,overlap;
        assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && present);
        assert(state.elapsed_us == elapsed && sample.graph_id == (graph == 7 ? 1 : 2));
        MvImageGraphic selected;
        assert(mv_process_image_get(ctx,actor,&selected,&present) == MV_OK && present);
        assert(selected.region.width == (graph == 7 ? 2 : 4));
        assert(mv_process_collider_test(ctx,actor,target,&overlap) == MV_OK && overlap == 1);
        mv_window_begin((MvColor){0,0,0,255});
        assert(mv_window_draw_process_graphics(ctx));
        Image screen=LoadImageFromScreen();
        Color pixel=GetImageColor(screen,20*screen.width/64,20*screen.height/64);
        assert(pixel.r == (graph == 7 ? 255 : 0) && pixel.g == (graph == 42 ? 255 : 0));
        UnloadImage(screen);
        mv_window_end();
        if (i == 4) {
            frames[1].duration_us = 0;
            assert(mv_process_image_animation_set(ctx,actor,frames,2,1,60) == MV_INVALID_ARGUMENT);
            frames[1].duration_us = 50000;
            assert(drops == 0);
        }
        if (i<6) assert(mv_process_tick(ctx) == MV_OK);
    }
    fclose(file);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_FREEZE,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    MvAnimationState state;
    MvAnimationSample sample;
    int present;
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && state.elapsed_us == 0);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_SLEEP,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && state.elapsed_us == 0);
    assert(mv_process_image_animation_set_owned(ctx,actor,frames,2,0,60,&drops,released) == MV_OK && drops == 1);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_WAKEUP,0) == MV_OK);
    for (int i=0;i<10;++i) assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK);
    assert(state.elapsed_us == 100000 && sample.index == 1);
    mv_window_begin((MvColor){0,0,0,255});
    assert(mv_window_draw_process_graphics(ctx));
    assert(mv_process_image_set(ctx,actor,frames[0].graphic) == MV_OK && drops == 2);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && !present);
    Image held=LoadImageFromScreen();
    Color final=GetImageColor(held,20*held.width/64,20*held.height/64);
    assert(final.g == 255 && final.r == 0);
    UnloadImage(held);
    mv_window_end();
    assert(mv_process_graphic_clear(ctx,actor) == MV_OK);
    red=mv_image_from_rgba(2,2,red_pixels,sizeof(red_pixels));
    assert(red);
    MvImageAnimationFrame shared[] = {{{red,{0,0,2,2},0,0},50000},{{red,{0,0,2,2},0.5,0.5},50000}};
    MvProcessRef foreign=spawn(other);
    assert(mv_process_image_animation_set_owned(ctx,actor,shared,2,1,60,&drops,released) == MV_OK);
    assert(mv_process_image_animation_set_owned(other,foreign,shared,2,1,60,&drops,released) == MV_OK);
    mv_image_destroy(red);
    int killed;
    assert(mv_process_kill(ctx,actor,&killed) == MV_OK && killed);
    assert(mv_process_tick(ctx) == MV_OK && drops == 3);
    MvProcessId ending;
    MvProcessRef returning;
    assert(mv_process_spawn(ctx,0,0,done,NULL,NULL,&ending) == MV_OK);
    assert(mv_process_ref_create(ctx,ending,&returning) == MV_OK);
    assert(mv_process_image_animation_set_owned(ctx,returning,shared,2,1,60,&drops,released) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK && drops == 4);
    mv_context_destroy(ctx);
    mv_context_destroy(other);
    assert(drops == 5);
    ctx=mv_context_create(60,4);
    assert(ctx);
    actor=spawn(ctx);
    red=mv_image_from_rgba(2,2,red_pixels,sizeof(red_pixels));
    assert(red);
    MvImageGraphic visual={red,{0,0,2,2},0.5,0.5};
    MvAnimationFrame duration={777,25000};
    MvAnimation *definition=mv_animation_create(&duration,1,0);
    assert(definition && mv_animation_count(definition) == 1);
    assert(mv_process_image_animation_bind_owned(ctx,actor,definition,&visual,2,60,&drops,released) == MV_INVALID_ARGUMENT);
    assert(drops == 5);
    assert(mv_process_image_animation_bind_owned(ctx,actor,definition,&visual,1,60,&drops,released) == MV_OK);
    mv_animation_destroy(definition);
    mv_image_destroy(red);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && present);
    assert(state.elapsed_us == 25000 && sample.graph_id == 777);
    mv_context_destroy(ctx);
    assert(drops == 6);

    mv_window_close();
    return 0;
}
