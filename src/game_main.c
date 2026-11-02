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
#include "game_directory.h"
#include "runner/runtime.h"

int
main(int argc, char **argv)
{
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "--headless") != 0)) {
        fprintf(stderr, "Usage: game [--headless]\n");
        return 2;
    }
    if (!malvado_enter_game_directory()) {
        fprintf(stderr, "Cannot open game directory\n");
        return 1;
    }
    if (argc == 2) {
        char *args[] = {argv[0], "--headless", "main.lua"};
        return malvado_run(3, args);
    }
    char *args[] = {argv[0], "main.lua"};
    return malvado_run(2, args);
}
