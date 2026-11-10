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
#include "globals.h"
#include "lauxlib.h"
#include "lualib.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { NAME_COUNT = 256 };
typedef struct { size_t calls, fail_at; } Allocator;

static void *
allocate(void *user, void *ptr, size_t old_size, size_t new_size)
{
    Allocator *allocator = user;
    if (!new_size) { free(ptr); return NULL; }
    if (new_size > old_size && allocator->calls++ >= allocator->fail_at) return NULL;
    return realloc(ptr, new_size);
}

static void
check_standard_conflict(void)
{
    static const char *const names[] = {"new_export", "print", NULL};
    lua_State *L = luaL_newstate();
    assert(L);
    luaL_openlibs(L);
    lua_newtable(L);
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "new_export");
    lua_pushinteger(L, 42);
    lua_setfield(L, -2, "print");
    lua_pushlightuserdata(L, (void *)names);
    lua_pushcclosure(L, malvado_import_globals, 2);
    int callback = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    assert(lua_pcall(L, 0, 0, 0) != LUA_OK);
    lua_pop(L, 1);
    lua_getglobal(L, "new_export");
    assert(lua_isnil(L, -1));
    lua_pop(L, 1);
    lua_getglobal(L, "print");
    assert(lua_isfunction(L, -1));
    lua_pop(L, 1);
    lua_pushinteger(L, 1);
    lua_rawseti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    assert(lua_pcall(L, 0, 0, 0) != LUA_OK);
    lua_pop(L, 1);
    lua_close(L);
}

int
main(void)
{
    char storage[NAME_COUNT][24];
    const char *names[NAME_COUNT + 1];
    for (int i = 0; i < NAME_COUNT; ++i) {
        snprintf(storage[i], sizeof(storage[i]), "test_export_%03d", i);
        names[i] = storage[i];
    }
    names[NAME_COUNT] = NULL;
    int failures = 0, later_failure = 0, successes = 0;
    for (size_t fail = 0; fail < 40; ++fail) {
        Allocator allocator = {0, SIZE_MAX};
        lua_State *L = lua_newstate(allocate, &allocator);
        assert(L);
        lua_newtable(L);
        for (int i = 0; i < NAME_COUNT; ++i) {
            lua_pushinteger(L, i + 1);
            lua_setfield(L, -2, names[i]);
        }
        lua_pushinteger(L, 1);
        lua_setglobal(L, names[0]);
        lua_pushvalue(L, -1);
        lua_pushlightuserdata(L, (void *)names);
        lua_pushcclosure(L, malvado_import_globals, 2);
        int callback = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pop(L, 1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
        allocator.calls = 0;
        allocator.fail_at = fail;
        int result = lua_pcall(L, 0, 0, 0);
        allocator.fail_at = SIZE_MAX;
        if (result == LUA_OK) ++successes;
        else {
            ++failures;
            if (fail >= 2) later_failure = 1;
            lua_pop(L, 1);
        }
        for (int i = 0; i < NAME_COUNT; ++i) {
            lua_getglobal(L, names[i]);
            if (i == 0 || result == LUA_OK) assert(lua_tointeger(L, -1) == i + 1);
            else assert(lua_isnil(L, -1));
            lua_pop(L, 1);
        }
        lua_close(L);
    }
    assert(failures && later_failure && successes);
    check_standard_conflict();
    return 0;
}
