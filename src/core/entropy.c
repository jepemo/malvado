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
#include "entropy.h"
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#include <limits.h>
#else
#include <stdio.h>
#endif

int
malvado_random_bytes(void *out, size_t size)
{
    if (!out || !size) return 0;
#ifdef _WIN32
    if (size > ULONG_MAX) return 0;
    return BCryptGenRandom(NULL, out, (ULONG)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
    FILE *source = fopen("/dev/urandom", "rb");
    if (!source) return 0;
    size_t count = fread(out, 1, size, source);
    int closed = fclose(source);
    return count == size && closed == 0;
#endif
}
