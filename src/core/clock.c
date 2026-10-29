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

#include "clock.h"
#include <math.h>

int malvado_clock_init(MalvadoClock *clock, double rate, unsigned max_steps) {
    if (!clock || !isfinite(rate) || rate <= 0 || max_steps == 0) return 0;
    clock->tick_seconds = 1.0 / rate;
    clock->remainder = 0;
    clock->max_steps = max_steps;
    clock->dropped_seconds = 0;
    return 1;
}

unsigned malvado_clock_advance(MalvadoClock *clock, double elapsed) {
    if (!clock || !isfinite(elapsed) || elapsed <= 0) return 0;
    clock->remainder += elapsed;
    unsigned steps = 0;
    while (clock->remainder + clock->tick_seconds * 1e-9 >= clock->tick_seconds && steps < clock->max_steps) {
        clock->remainder -= clock->tick_seconds;
        if (clock->remainder < 0) clock->remainder = 0;
        steps++;
    }
    if (clock->remainder >= clock->tick_seconds) {
        double remainder = fmod(clock->remainder, clock->tick_seconds);
        clock->dropped_seconds += clock->remainder - remainder;
        clock->remainder = remainder;
    }
    return steps;
}
