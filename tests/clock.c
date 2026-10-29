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
#include <assert.h>
#include <math.h>

int main(void) {
    MalvadoClock c;
    assert(!malvado_clock_init(&c, 0, 5));
    assert(malvado_clock_init(&c, 10, 5));
    assert(malvado_clock_advance(&c, 0.04) == 0);
    assert(malvado_clock_advance(&c, 0.06) == 1);
    assert(malvado_clock_advance(&c, 0.25) == 2);
    assert(malvado_clock_advance(&c, 0.05) == 1);
    assert(malvado_clock_advance(&c, 1.03) == 5);
    assert(fabs(c.dropped_seconds - 0.5) < 0.00001);
    assert(malvado_clock_advance(&c, -1) == 0);
    assert(malvado_clock_advance(&c, NAN) == 0);
    return 0;
}
