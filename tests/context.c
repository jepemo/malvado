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
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include "malvado/malvado.h"

int
main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *fixture = fopen(argv[1], "r");
    assert(fixture);
    MvContext *runner = mv_context_create(60,5);
    double elapsed, expected_dropped;
    unsigned expected_steps;
    unsigned long long expected_total;
    int rows = 0;
    while (fscanf(fixture, "%lf %u %llu %lf", &elapsed, &expected_steps,
                  &expected_total, &expected_dropped) == 4) {
        unsigned steps;
        assert(mv_context_advance(runner,elapsed,&steps) == MV_OK && steps == expected_steps);
        assert(mv_context_ticks(runner) == expected_total);
        assert(fabs(mv_context_dropped_seconds(runner)-expected_dropped) < 1e-9);
        ++rows;
    }
    assert(feof(fixture) && rows == 5);
    fclose(fixture);
    mv_context_destroy(runner);
    assert(!mv_context_create(0, 5));
    assert(!mv_context_create(-1, 5));
    assert(!mv_context_create(NAN, 5));
    assert(!mv_context_create(INFINITY, 5));
    /* A positive finite rate can still have an infinite reciprocal. */
    assert(!mv_context_create(DBL_MIN / 16, 5));
    MvContext *slow = mv_context_create(DBL_MIN, 5);
    assert(slow);
    mv_context_destroy(slow);
    assert(mv_context_ticks(NULL) == 0);
    assert(mv_context_dropped_seconds(NULL) == 0);
    assert(!mv_context_create(60, 0));
    MvContext *a = mv_context_create(10, 5);
    MvContext *b = mv_context_create(20, 5);
    assert(a && b);
    unsigned steps = 99;
    assert(mv_context_advance(a, 0.25, &steps) == MV_OK && steps == 2);
    assert(mv_context_ticks(a) == 2 && mv_context_ticks(b) == 0);
    assert(mv_context_advance(b, 0.1, &steps) == MV_OK && steps == 2);
    unsigned unchanged = steps;
    double dropped_before = mv_context_dropped_seconds(a);
    assert(mv_context_advance(NULL, 1, &steps) == MV_INVALID_ARGUMENT && steps == unchanged);
    assert(mv_context_advance(a, -1, &steps) == MV_INVALID_ARGUMENT && steps == unchanged);
    assert(mv_context_advance(a, NAN, &steps) == MV_INVALID_ARGUMENT && steps == unchanged);
    assert(mv_context_advance(a, INFINITY, &steps) == MV_INVALID_ARGUMENT && steps == unchanged);
    assert(mv_context_advance(a, 1, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_context_ticks(a) == 2 && mv_context_dropped_seconds(a) == dropped_before);
    assert(mv_context_advance(a, 0, &steps) == MV_OK && steps == 0);
    assert(mv_context_ticks(a) == 2 && mv_context_dropped_seconds(a) == dropped_before);
    assert(mv_context_advance(a, 1.03, &steps) == MV_OK && steps == 5);
    assert(mv_context_dropped_seconds(a) > 0.4);
    MvContext *overflow = mv_context_create(10, 5);
    assert(overflow);
    assert(mv_context_advance(overflow, DBL_MAX, &steps) == MV_OK);
    uint64_t before = mv_context_ticks(overflow);
    unsigned previous_steps = steps;
    double previous_dropped = mv_context_dropped_seconds(overflow);
    assert(mv_context_advance(overflow, DBL_MAX, &steps) == MV_OVERFLOW);
    assert(mv_context_ticks(overflow) == before && steps == previous_steps);
    assert(mv_context_dropped_seconds(overflow) == previous_dropped);
    mv_context_destroy(overflow);
    /* A tick is too small to change DBL_MAX. Even a large step limit must
     * finish promptly and preserve the public clock accounting. */
    MvContext *large = mv_context_create(60, UINT_MAX);
    assert(large);
    assert(mv_context_advance(large, DBL_MAX, &steps) == MV_OK);
    assert(steps == UINT_MAX && mv_context_ticks(large) == UINT_MAX);
    assert(isfinite(mv_context_dropped_seconds(large)) &&
           mv_context_dropped_seconds(large) > 0);
    double large_dropped = mv_context_dropped_seconds(large);
    steps = 123;
    assert(mv_context_advance(large, DBL_MAX, &steps) == MV_OVERFLOW);
    assert(steps == 123 && mv_context_ticks(large) == UINT_MAX);
    assert(mv_context_dropped_seconds(large) == large_dropped);
    mv_context_destroy(large);
    mv_context_destroy(a);
    mv_context_destroy(b);
    mv_context_destroy(NULL);
    return 0;
}
