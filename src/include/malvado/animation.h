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
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvAnimation MvAnimation;
typedef struct { int graph_id; uint64_t duration_us; } MvAnimationFrame;
typedef struct { size_t index; int graph_id; } MvAnimationSample;
typedef struct { uint64_t elapsed_us; unsigned remainder, tick_rate; } MvAnimationState;

/* Copy 1..8192 frames into an immutable owned definition. IDs must be positive,
 * durations 1..3600000000 microseconds, loop 0 or 1. Return NULL on invalid input
 * or allocation failure. Destroy only after all callers stop using it. */
MvAnimation *mv_animation_create(const MvAnimationFrame *frames, size_t count, int loop);
void mv_animation_destroy(MvAnimation *animation);
/* Sample a zero-based sequence index. Non-looping sequences hold the last
 * frame. Return 1, or 0 without changing output on invalid pointers. */
int mv_animation_sample(const MvAnimation *animation, uint64_t elapsed_us, MvAnimationSample *out);
/* Start state at {0}. Advance one simulation tick at a fixed rate 1..1000000.
 * A rate change requires resetting state. Looping state stays bounded; stopped
 * sequences hold their end. Return 0 without changes on invalid state/rate. */
int mv_animation_advance(const MvAnimation *animation, MvAnimationState *state, unsigned tick_rate);

#ifdef __cplusplus
}
#endif
#endif
