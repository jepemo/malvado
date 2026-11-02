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
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir_project(path) _mkdir(path)
#define remove_project(path) _rmdir(path)
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define mkdir_project(path) mkdir(path, 0777)
#define remove_project(path) rmdir(path)
#endif
#include "lua.h"
#include "lauxlib.h"
#include "runner/runtime.h"
#include "export.h"

static int
check_script(const char *path)
{
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Cannot create Lua state\n");
        return 1;
    }
    int result = luaL_loadfilex(L, path, "t");
    if (result != LUA_OK)
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
    else
        printf("Syntax OK: %s\n", path);
    lua_close(L);
    return result == LUA_OK ? 0 : 1;
}

static int check_directory(const char *path, unsigned depth, unsigned *files, int for_export);

static int
check_entry(const char *directory, const char *name, int is_directory,
            unsigned depth, unsigned *files, int for_export)
{
    if (name[0] == '.')
        return 0;
    if (for_export && is_directory && (!strcmp(name, "build") || !strcmp(name, "dist")))
        return 0;
    size_t length = strlen(directory) + strlen(name) + 2;
    char *path = malloc(length);
    if (!path) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    snprintf(path, length, "%s/%s", directory, name);
    int result = 0;
    if (is_directory) {
        result = check_directory(path, depth + 1, files, for_export);
    } else {
        size_t name_length = strlen(name);
        if (name_length >= 4 && strcmp(name + name_length - 4, ".lua") == 0) {
            ++*files;
            result = check_script(path);
        }
    }
    free(path);
    return result;
}

static int
check_directory(const char *path, unsigned depth, unsigned *files, int for_export)
{
    if (depth > 64) {
        fprintf(stderr, "Directory nesting exceeds 64: %s\n", path);
        return 1;
    }
    int failed = 0;
#ifdef _WIN32
    size_t length = strlen(path) + 3;
    char *pattern = malloc(length);
    if (!pattern) return 1;
    snprintf(pattern, length, "%s/*", path);
    WIN32_FIND_DATAA entry;
    HANDLE handle = FindFirstFileA(pattern, &entry);
    free(pattern);
    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Cannot read directory: %s\n", path);
        return 1;
    }
    do {
        if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
            failed |= check_entry(path, entry.cFileName,
                (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, depth, files, for_export);
    } while (FindNextFileA(handle, &entry));
    if (GetLastError() != ERROR_NO_MORE_FILES) failed = 1;
    FindClose(handle);
#else
    DIR *directory = opendir(path);
    if (!directory) {
        perror(path);
        return 1;
    }
    struct dirent *entry;
    for (;;) {
        errno = 0;
        entry = readdir(directory);
        if (!entry) {
            if (errno != 0) { perror(path); failed = 1; }
            break;
        }
        if (entry->d_name[0] == '.') continue;
        size_t length = strlen(path) + strlen(entry->d_name) + 2;
        char *full = malloc(length);
        if (!full) { failed = 1; break; }
        snprintf(full, length, "%s/%s", path, entry->d_name);
        struct stat info;
        if (lstat(full, &info) != 0) {
            perror(full);
            failed = 1;
        } else if (S_ISREG(info.st_mode) || S_ISDIR(info.st_mode)) {
            failed |= check_entry(path, entry->d_name, S_ISDIR(info.st_mode), depth, files, for_export);
        }
        free(full);
    }
    closedir(directory);
#endif
    return failed;
}

static int
check_project(int for_export)
{
    unsigned files = 0;
    int failed = check_directory(".", 0, &files, for_export);
    if (!files) {
        fprintf(stderr, "No Lua files found\n");
        return 1;
    }
    return failed ? 1 : 0;
}

static int
new_project(const char *path)
{
    static const char game[] =
        "require(\"malvado\").import_globals()\n\n"
        "local Game = process(function()\n"
        "  while not key(\"escape\") do\n"
        "    draw_text(\"Hello, Malvado!\", 40, 40, 30)\n"
        "    frame()\n"
        "  end\n"
        "end)\n\n"
        "start(function() Game() end)\n";
    size_t length = strlen(path);
    if (!length || length > 4096) {
        fprintf(stderr, "Invalid project path\n");
        return 2;
    }
    char *file_path = malloc(length + sizeof("/main.lua"));
    if (!file_path) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    snprintf(file_path, length + sizeof("/main.lua"), "%s/main.lua", path);
    if (mkdir_project(path) != 0) {
        perror("Cannot create project directory");
        free(file_path);
        return 1;
    }
    FILE *file = fopen(file_path, "wx");
    if (!file) {
        perror("Cannot create main.lua");
        remove_project(path);
        free(file_path);
        return 1;
    }
    int failed = fwrite(game, 1, sizeof(game) - 1, file) != sizeof(game) - 1;
    if (fclose(file) != 0) failed = 1;
    if (failed) {
        fprintf(stderr, "Cannot write main.lua\n");
        remove(file_path);
        remove_project(path);
        free(file_path);
        return 1;
    }
    printf("Created %s. Enter the project directory and run: malvado run\n", file_path);
    free(file_path);
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "doctor") == 0) {
        printf("Lua: %s\n", LUA_RELEASE);
#ifdef MALVADO_GRAPHICS_ENABLED
        printf("Graphics: enabled\n");
#else
        printf("Graphics: headless\n");
#endif
        int status = malvado_export_status();
        if (status == 2)
            printf("Export: launcher and license directory found\n");
        else if (status == 1)
            printf("Export: launcher or license directory not found; install the SDK\n");
        else
            printf("Export: unavailable in this build\n");
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "new") == 0)
        return new_project(argv[2]);
    if (argc == 3 && strcmp(argv[1], "export") == 0) {
        if (check_project(1) != 0) return 1;
        return malvado_export(argv[2]);
    }
    if (argc >= 2 && argc <= 3 && strcmp(argv[1], "check") == 0)
        return argc == 3 ? check_script(argv[2]) : check_project(0);
    if (argc >= 2 && strcmp(argv[1], "run") == 0) {
        if (argc == 2) {
            char *args[] = {argv[0], "main.lua"};
            return malvado_run(2, args);
        }
        if (argc == 3) {
            if (strcmp(argv[2], "--headless") == 0) {
                char *args[] = {argv[0], argv[2], "main.lua"};
                return malvado_run(3, args);
            }
            char *args[] = {argv[0], argv[2]};
            return malvado_run(2, args);
        }
        if (argc == 4 && strcmp(argv[2], "--headless") == 0) {
            char *args[] = {argv[0], argv[2], argv[3]};
            return malvado_run(3, args);
        }
    }
    fprintf(stderr, "Usage: malvado new PATH | malvado run [--headless] [FILE] | malvado check [FILE] | malvado export PATH | malvado doctor\n");
    return 2;
}
