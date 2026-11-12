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
#include "runner/runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static double times[6];
static unsigned time_index, callbacks;
double test_runner_time(void)
{
    assert(time_index < 6);
    return times[time_index++];
}
static void check_tick(void *user)
{
    (void)user;
    const uint64_t planned[] = {0,1,6,6,6,6,6,7};
    assert(callbacks < 8);
    MvContext *ctx = malvado_debug_context();
    assert(mv_context_ticks(ctx) == planned[callbacks]);
    double expected = callbacks >= 2 ? 0.9 : 0;
    assert(fabs(mv_context_dropped_seconds(ctx)-expected) < 1e-9);
    ++callbacks;
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    FILE *fixture = fopen(argv[2],"r");
    assert(fixture);
    double elapsed, dropped;
    unsigned steps;
    unsigned long long total;
    unsigned rows = 0;
    while (fscanf(fixture,"%lf %u %llu %lf",&elapsed,&steps,&total,&dropped) == 4) {
        assert(rows < 5);
        times[rows+1] = times[rows]+elapsed;
        ++rows;
    }
    assert(feof(fixture) && rows == 5);
    fclose(fixture);
    char *args[] = {"malvado-run",argv[1]};
    malvado_debug_set_hook(check_tick,NULL);
    assert(malvado_run(2,args) == 0);
    malvado_debug_set_hook(NULL,NULL);
    assert(callbacks == 8 && time_index == 6);
    return 0;
}
