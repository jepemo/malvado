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
#include <stdio.h>
#include <string.h>
#include "malvado/platform.h"

int
main(void)
{
    const char text[] = "A\xc3\xb1";
    size_t offset = 0;
    while (offset < sizeof(text) - 1) {
        uint32_t scalar;
        size_t count = mv_utf8_decode(text + offset, sizeof(text) - 1 - offset, &scalar);
        if (!count)
            return 1;
        char encoded[4];
        size_t encoded_count = mv_utf8_encode(scalar, encoded);
        if (encoded_count != count || memcmp(encoded, text + offset, count) != 0)
            return 1;
        printf("U+%04X\n", (unsigned)scalar);
        offset += count;
    }
    return 0;
}
