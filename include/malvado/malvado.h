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

#ifndef MALVADO_H
#define MALVADO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvContext MvContext;

typedef enum {
    MV_OK = 0,
    MV_INVALID_ARGUMENT,
    MV_OVERFLOW
} MvResult;

/** Create an independent context and logical clock.
 * @param ticks_per_second Finite, positive logical tick rate in hertz.
 * @param max_steps Maximum ticks returned for one advance; must be nonzero.
 * @return Owned context, or NULL for invalid settings or allocation failure.
 * Destroy the context with mv_context_destroy. */
MvContext *mv_context_create(double ticks_per_second, unsigned max_steps);
void mv_context_destroy(MvContext *ctx);

/** Advance the logical clock without reading the system clock.
 * @param ctx Context created by mv_context_create; must not be NULL.
 * @param elapsed_seconds Finite, nonnegative duration in seconds.
 * @param steps Receives the number of ticks; must not be NULL.
 * @return MV_OK, MV_INVALID_ARGUMENT or MV_OVERFLOW. On failure the context
 * and steps are unchanged. Time above max_steps is recorded as dropped. */
MvResult mv_context_advance(MvContext *ctx, double elapsed_seconds, unsigned *steps);
uint64_t mv_context_ticks(const MvContext *ctx);
double mv_context_dropped_seconds(const MvContext *ctx);

#ifdef __cplusplus
}
#endif
#endif
