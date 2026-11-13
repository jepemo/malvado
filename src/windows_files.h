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
#ifndef MALVADO_WINDOWS_FILES_H
#define MALVADO_WINDOWS_FILES_H
#include <windows.h>
#include <sys/stat.h>
#define MALVADO_CLI_PATH_MAX 4096
typedef struct {
    HANDLE handle;
    WIN32_FIND_DATAA entry;
    int first;
} MalvadoWindowsDirectory;
int malvado_windows_lstat(const char *path, struct stat *info);
char *malvado_windows_realpath(const char *path, char *resolved);
MalvadoWindowsDirectory *malvado_windows_directory_open(const char *path);
const char *malvado_windows_directory_next(MalvadoWindowsDirectory *directory);
int malvado_windows_directory_close(MalvadoWindowsDirectory *directory);
char *malvado_windows_stage(const char *parent);
int malvado_windows_remove_tree(const char *path);
int malvado_windows_publish(const char *stage, const char *output);
#endif
