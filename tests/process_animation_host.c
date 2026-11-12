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
#include "runner/runtime.h"
#include "malvado/animation.h"

typedef struct { FILE *trace; int stage; } Probe;
static void between_ticks(void *user)
{
    Probe *probe = user;
    if (probe->stage >= 7) return;
    MvContext *ctx = malvado_debug_context();
    MvProcessRef ref;
    if (mv_process_ref_create(ctx,1,&ref) != MV_OK) return;
    MvAnimationState state;
    MvAnimationSample sample;
    int present;
    MvResult result = mv_process_animation_clock_get(ctx,ref,&state,&sample,&present);
    assert(result == MV_OK && present);
    if (probe->stage < 6) {
        unsigned long long elapsed;
        int graph;
        assert(fscanf(probe->trace,"%llu %d",&elapsed,&graph) == 2);
        assert(state.elapsed_us == elapsed && sample.graph_id == graph);
        if (probe->stage == 5)
            assert(mv_process_animation_clock_clear(ctx,ref) == MV_OK);
    } else {
        /* Lua reassigns its same descriptor after the native clear. */
        assert(state.elapsed_us == 0);
        int killed;
        assert(mv_process_kill(ctx,ref,&killed) == MV_OK && killed);
    }
    ++probe->stage;
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    Probe probe = {fopen(argv[2],"r"),0};
    assert(probe.trace);
    unsigned long long initial;
    int graph;
    assert(fscanf(probe.trace,"%llu %d",&initial,&graph) == 2 && initial == 0 && graph == 7);
    char *args[] = {"malvado-run","--headless",argv[1]};
    malvado_debug_set_hook(between_ticks,&probe);
    assert(malvado_run(3,args) == 0);
    malvado_debug_set_hook(NULL,NULL);
    fclose(probe.trace);
    assert(probe.stage == 7);
    return 0;
}
