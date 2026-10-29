/* Malvado 2 - Jeremías Pérez and contributors. SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include "malvado/malvado.h"

int
main(void)
{
    assert(!mv_context_create(0, 5));
    assert(!mv_context_create(60, 0));
    MvContext *a = mv_context_create(10, 5);
    MvContext *b = mv_context_create(20, 5);
    assert(a && b);
    unsigned steps = 99;
    assert(mv_context_advance(a, 0.25, &steps) == MV_OK && steps == 2);
    assert(mv_context_ticks(a) == 2 && mv_context_ticks(b) == 0);
    assert(mv_context_advance(b, 0.1, &steps) == MV_OK && steps == 2);
    assert(mv_context_advance(a, -1, &steps) == MV_INVALID_ARGUMENT);
    assert(mv_context_advance(a, NAN, &steps) == MV_INVALID_ARGUMENT);
    assert(mv_context_advance(a, 1, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_context_ticks(a) == 2);
    assert(mv_context_advance(a, 1.03, &steps) == MV_OK && steps == 5);
    assert(mv_context_dropped_seconds(a) > 0.4);
    MvContext *overflow = mv_context_create(10, 5);
    assert(overflow);
    assert(mv_context_advance(overflow, DBL_MAX, &steps) == MV_OK);
    uint64_t before = mv_context_ticks(overflow);
    unsigned previous_steps = steps;
    assert(mv_context_advance(overflow, DBL_MAX, &steps) == MV_OVERFLOW);
    assert(mv_context_ticks(overflow) == before && steps == previous_steps);
    mv_context_destroy(overflow);
    mv_context_destroy(a);
    mv_context_destroy(b);
    mv_context_destroy(NULL);
    return 0;
}
