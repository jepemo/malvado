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
#include "malvado/platform.h"

int
main(void)
{
    double before = mv_platform_time();
    mv_platform_wait(0.04);
    double after = mv_platform_time();
    assert(after >= before);
    assert(after - before >= 0.02);
    return 0;
}
