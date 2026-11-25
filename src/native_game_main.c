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

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
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
    char directory[8192];
    if (!_getcwd(directory, sizeof(directory))) return 1;
    return malvado_windows_process_with_game_dir(argv, NULL, directory, NULL, 0);
#else
    argv[0] = "./game-bin";
    char *directory = getcwd(NULL, 0);
    if (!directory || setenv("MALVADO_GAME_DIR", directory, 1) != 0) {
        free(directory);
        return 1;
    }
    free(directory);
    execv(argv[0], argv);
#endif
    perror("game-bin");
    return 1;
}
