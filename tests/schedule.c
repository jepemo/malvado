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
#include <assert.h>
#include <stdint.h>
#include "schedule.h"
#include "malvado/malvado.h"

int
main(void)
{
    MalvadoScheduleItem items[] = {
        {0, 2}, {1, INT64_MIN}, {2, INT64_MAX}, {3, 2}, {4, 0}
    };
    malvado_schedule_sort(items, 5);
    assert(items[0].slot == 2 && items[1].slot == 0 && items[2].slot == 3);
    assert(items[3].slot == 4 && items[4].slot == 1);
    malvado_schedule_sort(NULL, 0);
    MvCadence cadence = {0};
    int due = 0;
    for (int tick = 0; tick < 60; ++tick)
        due += mv_cadence_step(&cadence, 17);
    assert(due == 17 && cadence.phase == 0);
    assert(mv_cadence_step(&cadence, 15) == 0 && cadence.phase == 15);
    assert(mv_cadence_step(&cadence, 61) == -1 && cadence.phase == 15);
    assert(mv_cadence_step(NULL, 15) == -1);
    assert(mv_cadence_step(&cadence, 0) == 1 && cadence.phase == 0);
    cadence.phase = 60;
    assert(mv_cadence_step(&cadence, 15) == -1 && cadence.phase == 60);
    return 0;
}
