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

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define change_directory _chdir
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#define change_directory chdir
#else
#include <unistd.h>
#define change_directory chdir
#endif
#include "game_directory.h"

int
malvado_enter_game_directory(void)
{
    char path[8192];
#ifdef _WIN32
    DWORD length = GetModuleFileNameA(NULL, path, (DWORD)sizeof(path));
    if (length == 0 || length >= sizeof(path)) return 0;
    char *separator = strrchr(path, '\\');
#elif defined(__APPLE__)
    uint32_t length = sizeof(path);
    if (_NSGetExecutablePath(path, &length) != 0) return 0;
    char *separator = strrchr(path, '/');
#else
    ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length <= 0 || (size_t)length >= sizeof(path) - 1) return 0;
    path[length] = '\0';
    char *separator = strrchr(path, '/');
#endif
    if (!separator) return 0;
    if (separator == path) separator[1] = '\0';
    else *separator = '\0';
    return change_directory(path) == 0;
}

