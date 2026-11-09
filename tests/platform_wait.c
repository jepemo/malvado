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
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include "malvado/platform.h"

int
main(void)
{
    mv_platform_wait(NAN);
    mv_platform_wait(INFINITY);
    mv_platform_wait(DBL_MAX);
#ifdef _WIN32
    mv_platform_wait((double)UINT32_MAX / 1000.0);
#else
    mv_platform_wait((double)INT_MAX);
#endif
    mv_platform_wait(-1);
    mv_platform_wait(0);
    double before, after;
    assert(mv_monotonic_time(&before));
    double platform_before = mv_platform_time();
    mv_platform_wait(0.04);
    double platform_after = mv_platform_time();
    assert(mv_monotonic_time(&after));
    assert(platform_after >= platform_before);
    assert(platform_after - platform_before >= 0.02);
    assert(after - before >= 0.02);
    return 0;
}
