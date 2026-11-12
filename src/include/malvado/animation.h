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

#ifndef MALVADO_ANIMATION_H
#define MALVADO_ANIMATION_H
#include <stddef.h>
#include <stdint.h>
#include "malvado/malvado.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvAnimation MvAnimation;
typedef struct { int graph_id; uint64_t duration_us; } MvAnimationFrame;
typedef struct { size_t index; int graph_id; } MvAnimationSample;
typedef struct { uint64_t elapsed_us; unsigned remainder, tick_rate; } MvAnimationState;

/* Copy 1..8192 frames into an immutable owned definition. IDs must be positive,
 * durations 1..3600000000 microseconds, loop 0 or 1. Return NULL on invalid input
 * or allocation failure. Each owned reference needs one destroy call. */
MvAnimation *mv_animation_create(const MvAnimationFrame *frames, size_t count, int loop);
/* Acquire an independent reference. NULL/overflow returns 0 unchanged.
 * Reference acquisition/release must stay on the owner thread. */
int mv_animation_retain(MvAnimation *animation);
void mv_animation_destroy(MvAnimation *animation);
/* Sample a zero-based sequence index. Non-looping sequences hold the last
 * frame. Return 1, or 0 without changing output on invalid pointers. */
int mv_animation_sample(const MvAnimation *animation, uint64_t elapsed_us, MvAnimationSample *out);
/* Start state at {0}. Advance one simulation tick at a fixed rate 1..1000000.
 * A rate change requires resetting state. Looping state stays bounded; stopped
 * sequences hold their end. Return 0 without changes on invalid state/rate. */
int mv_animation_advance(const MvAnimation *animation, MvAnimationState *state, unsigned tick_rate);

/* Process-owned playback clock. Set retains the definition and resets state.
 * Rate is explicit: logical ticks per second, 1..1000000. It must describe the
 * host's simulation rate; no timer or second scheduler is created.
 * Advance once at the end of each native process tick while running, including
 * cadence-skipped activations. Sleep/freeze pause without catch-up. Assignment
 * during a tick skips that tick; assignment outside a tick advances on the next.
 * Get copies state/sample only when present; absent leaves both unchanged.
 * All outputs are required and must not overlap. Clear is idempotent. Failure
 * preserves state/outputs. Foreign/invalid inputs return MV_INVALID_ARGUMENT;
 * ended processes return MV_NOT_FOUND. Owner-thread only.
 * Graph assignment/clear does not stop this independent clock. Fractional host
 * tick rates are unsupported; choose an integer simulation frequency.
 * Graph IDs are metadata: no graph assignment, assets or rendering is implicit.
 * Clear/replacement, natural return, kill and context shutdown release the clock. */
MvResult mv_process_animation_clock_set(MvContext *ctx, MvProcessRef process,
                                  MvAnimation *animation, unsigned tick_rate);
MvResult mv_process_animation_clock_get(MvContext *ctx, MvProcessRef process,
                                  MvAnimationState *state, MvAnimationSample *sample, int *present);
MvResult mv_process_animation_clock_clear(MvContext *ctx, MvProcessRef process);

#ifdef __cplusplus
}
#endif
#endif
