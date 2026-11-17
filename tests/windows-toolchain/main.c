/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#include <stdio.h>

#if !defined(_M_X64) || !defined(_WIN64) || _MSC_FULL_VER != 194435230
#error The probe requires the pinned x64 MSVC compiler.
#endif
_Static_assert(sizeof(void *) == 8, "The probe requires x64 pointers.");

int
main(void)
{
    puts("Windows toolchain probe passed");
    return 0;
}
