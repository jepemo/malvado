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
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir_project(path) _mkdir(path)
#define remove_project(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define mkdir_project(path) mkdir(path, 0777)
#define remove_project(path) rmdir(path)
#endif
#include "lua.h"
#include "lauxlib.h"
#include "runner/runtime.h"

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
    if (argc == 3 && strcmp(argv[1], "new") == 0)
        return new_project(argv[2]);
    if (argc >= 2 && argc <= 3 && strcmp(argv[1], "check") == 0)
        return check_script(argc == 3 ? argv[2] : "main.lua");
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
    fprintf(stderr, "Usage: malvado new PATH | malvado run [--headless] [FILE] | malvado check [FILE]\n");
    return 2;
}
