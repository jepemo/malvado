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
#include <limits.h>
#include <math.h>

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

double
mv_platform_time(void)
{
    double seconds;
    return mv_monotonic_time(&seconds) ? seconds : 0;
}

void
mv_platform_wait(double seconds)
{
    if (!isfinite(seconds) || seconds <= 0)
        return;
#ifdef _WIN32
    /* UINT32_MAX is the infinite wait sentinel for Sleep. */
    if (seconds >= (double)UINT32_MAX / 1000.0)
        return;
    Sleep((DWORD)(seconds * 1000));
#else
    /* Keep the conversion representable even with a 32-bit time_t. */
    if (seconds >= (double)INT_MAX)
        return;
    struct timespec duration = {(time_t)seconds, (long)((seconds - (time_t)seconds) * 1e9)};
    nanosleep(&duration, NULL);
#endif
}
