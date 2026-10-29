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

#include "schedule.h"
#include <stdlib.h>

static int
compare_items(const void *a, const void *b)
{
    const MalvadoScheduleItem *left = a;
    const MalvadoScheduleItem *right = b;
    if (left->priority != right->priority)
        return left->priority > right->priority ? -1 : 1;
    if (left->slot == right->slot)
        return 0;
    return left->slot < right->slot ? -1 : 1;
}

void
malvado_schedule_sort(MalvadoScheduleItem *items, size_t count)
{
    if (count > 1)
        qsort(items, count, sizeof(*items), compare_items);
}
