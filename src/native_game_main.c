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
#ifdef _WIN32
#include "windows_process.h"
#else
#include <unistd.h>
#endif
#include "game_directory.h"

int
main(int argc, char **argv)
{
    (void)argc;
    if (!malvado_enter_game_directory()) {
        fprintf(stderr, "Cannot open game directory\n");
        return 1;
    }
#ifdef _WIN32
    argv[0] = "./game-bin.exe";
    return malvado_windows_process(argv, NULL, NULL, 0);
#else
    argv[0] = "./game-bin";
    execv(argv[0], argv);
#endif
    perror("game-bin");
    return 1;
}
