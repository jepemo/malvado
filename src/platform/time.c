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

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#else
#include <windows.h>
#endif
#include "malvado/platform.h"

int
mv_monotonic_time(double *seconds)
{
    if (!seconds)
        return 0;
#ifdef _WIN32
    LARGE_INTEGER counter, frequency;
    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0 ||
        !QueryPerformanceCounter(&counter))
        return 0;
    *seconds = (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0)
        return 0;
    *seconds = (double)value.tv_sec + (double)value.tv_nsec / 1e9;
#endif
    return 1;
}
