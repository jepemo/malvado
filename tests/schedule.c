/*
 * Malvado - game development with Lua and processes.
 * Project created by Jeremías Pérez; Malvado contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */
#include <assert.h>
#include <stdint.h>
#include "schedule.h"

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
    return 0;
}
