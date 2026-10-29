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

#ifndef MALVADO_CLOCK_H
#define MALVADO_CLOCK_H

typedef struct {
    double tick_seconds;
    double remainder;
    unsigned max_steps;
    double dropped_seconds;
} MalvadoClock;

int malvado_clock_init(MalvadoClock *clock, double ticks_per_second, unsigned max_steps);
unsigned malvado_clock_advance(MalvadoClock *clock, double elapsed_seconds);

#endif
