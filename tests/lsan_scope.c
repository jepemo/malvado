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
/* Verify that the graphics leak filter excludes only its named stack. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
extensionSupportedGLX(void)
{
    void *data = malloc(32);
    fprintf(stderr, "suppressed fixture: %p\n", data);
}

void
owned_allocation(void)
{
    void *data = malloc(32);
    fprintf(stderr, "visible fixture: %p\n", data);
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "glx") == 0)
        extensionSupportedGLX();
    else
        owned_allocation();
    return 0;
}
