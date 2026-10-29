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

#ifndef MALVADO_SCHEDULE_H
#define MALVADO_SCHEDULE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t slot;
    int64_t priority;
} MalvadoScheduleItem;

/* Higher priority first; ties retain the order of the original slots. */
void malvado_schedule_sort(MalvadoScheduleItem *items, size_t count);

#endif
