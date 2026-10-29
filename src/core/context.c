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

#include "malvado/malvado.h"
#include "clock.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

struct MvContext {
    MalvadoClock clock;
    uint64_t ticks;
};

MvContext *
mv_context_create(double ticks_per_second, unsigned max_steps)
{
    if (!isfinite(ticks_per_second) || ticks_per_second <= 0 ||
        !isfinite(1.0 / ticks_per_second) || 1.0 / ticks_per_second <= 0)
        return NULL;
    MvContext *ctx = malloc(sizeof(*ctx));
    if (!ctx)
        return NULL;
    if (!malvado_clock_init(&ctx->clock, ticks_per_second, max_steps)) {
        free(ctx);
        return NULL;
    }
    ctx->ticks = 0;
    return ctx;
}

void
mv_context_destroy(MvContext *ctx)
{
    free(ctx);
}

MvResult
mv_context_advance(MvContext *ctx, double elapsed_seconds, unsigned *steps)
{
    if (!ctx || !steps || !isfinite(elapsed_seconds) || elapsed_seconds < 0)
        return MV_INVALID_ARGUMENT;

    /* Compute on a copy so a failed operation never changes the context. */
    MalvadoClock clock = ctx->clock;
    unsigned count = malvado_clock_advance(&clock, elapsed_seconds);
    if (!isfinite(clock.remainder) || !isfinite(clock.dropped_seconds) ||
        UINT64_MAX - ctx->ticks < count)
        return MV_OVERFLOW;
    ctx->clock = clock;
    ctx->ticks += count;
    *steps = count;
    return MV_OK;
}

uint64_t
mv_context_ticks(const MvContext *ctx)
{
    return ctx ? ctx->ticks : 0;
}

double
mv_context_dropped_seconds(const MvContext *ctx)
{
    return ctx ? ctx->clock.dropped_seconds : 0;
}
