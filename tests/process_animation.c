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
#include "malvado/animation.h"

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
static MvStepResult attach(MvContext *ctx, MvProcessId id, void *user)
{
    MvProcessRef ref;
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    assert(mv_process_animation_clock_set(ctx, ref, user, 60) == MV_OK);
    return MV_STEP_YIELD;
}
static MvProcessRef spawn(MvContext *ctx, MvProcessStep step, void *user)
{
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx, 0, 0, step, NULL, user, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &ref) == MV_OK);
    return ref;
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    MvContext *ctx = mv_context_create(60,4), *other = mv_context_create(60,4);
    assert(ctx && other);
    MvProcessRef actor = spawn(ctx, idle, NULL);
    MvAnimationFrame frames[] = {{7,50000},{42,50000}};
    MvAnimation *animation = mv_animation_create(frames,2,1);
    assert(animation && mv_animation_retain(animation));
    mv_animation_destroy(animation);
    assert(!mv_animation_retain(NULL));
    MvAnimationState state = {99,0,0};
    MvAnimationSample sample = {99,99};
    int present = 9;
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && !present);
    assert(state.elapsed_us == 99 && sample.graph_id == 99);
    assert(mv_process_animation_clock_set(other,actor,animation,60) == MV_INVALID_ARGUMENT);
    assert(mv_process_animation_clock_set(ctx,actor,NULL,60) == MV_INVALID_ARGUMENT);
    assert(mv_process_animation_clock_set(ctx,actor,animation,0) == MV_INVALID_ARGUMENT);
    assert(mv_process_animation_clock_set(ctx,actor,animation,1000001) == MV_INVALID_ARGUMENT);
    assert(mv_process_animation_clock_set(ctx,actor,animation,60) == MV_OK);
    mv_animation_destroy(animation); /* Process now owns the complete definition. */
    FILE *file = fopen(argv[1],"r");
    assert(file);
    for (int i = 0; i <= 6; ++i) {
        unsigned long long elapsed;
        int graph;
        assert(fscanf(file,"%llu %d",&elapsed,&graph) == 2);
        assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && present);
        assert(state.elapsed_us == elapsed && sample.graph_id == graph);
        if (i < 6) assert(mv_process_tick(ctx) == MV_OK);
    }
    fclose(file);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_SLEEP,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && state.elapsed_us == 0);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_FREEZE,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && state.elapsed_us == 0);
    assert(mv_process_signal(ctx,actor.id,MV_SIGNAL_WAKEUP,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK && state.elapsed_us == 16666);
    assert(mv_process_animation_clock_get(ctx,actor,NULL,&sample,&present) == MV_INVALID_ARGUMENT);
    assert(mv_process_animation_clock_clear(ctx,actor) == MV_OK);
    assert(mv_process_animation_clock_clear(ctx,actor) == MV_OK);
    animation = mv_animation_create(frames,2,0);
    MvProcessRef resetting = spawn(ctx,attach,animation);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,resetting,&state,&sample,&present) == MV_OK && state.elapsed_us == 0);
    /* A separate process holds the last frame of a non-looping sequence. */
    assert(mv_process_animation_clock_set(ctx,actor,animation,60) == MV_OK);
    mv_animation_destroy(animation);
    for (int i=0;i<10;++i) assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_OK);
    assert(state.elapsed_us == 100000 && sample.graph_id == 42);
    int killed;
    assert(mv_process_kill(ctx,actor,&killed) == MV_OK && killed);
    assert(mv_process_animation_clock_get(ctx,actor,&state,&sample,&present) == MV_NOT_FOUND);
    assert(mv_process_tick(ctx) == MV_OK);
    mv_context_destroy(ctx);
    mv_context_destroy(other);
    return 0;
}
