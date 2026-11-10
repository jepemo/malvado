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
#include <stdlib.h>

typedef struct {
    const char *const *names;
    unsigned char *inserted;
} ImportState;

static int
install_globals(lua_State *L)
{
    ImportState *state = lua_touserdata(L, 3);
    for (size_t i = 0; state->names[i]; ++i) {
        const char *name = state->names[i];
        lua_pushstring(L, name);
        lua_rawget(L, 1);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushstring(L, name);
        lua_rawget(L, 2);
        if (!lua_isnil(L, -1)) {
            int equal = lua_rawequal(L, -1, -2);
            lua_pop(L, 2);
            if (!equal) return luaL_error(L, "global already exists: %s", name);
            continue;
        }
        lua_pop(L, 1);
        lua_pushstring(L, name);
        lua_pushvalue(L, -2);
        lua_rawset(L, 2);
        state->inserted[i] = 1;
        lua_pop(L, 1);
    }
    return 0;
}

int
malvado_import_globals(lua_State *L)
{
    const char *const *names = lua_touserdata(L, lua_upvalueindex(2));
    lua_pushglobaltable(L);
    int globals = lua_gettop(L);
    if (!lua_istable(L, globals)) return luaL_error(L, "global environment is not a table");
    if (!lua_checkstack(L, 8)) return luaL_error(L, "cannot prepare global import");
    for (size_t i = 0; names[i]; ++i) {
        lua_pushstring(L, names[i]);
        lua_rawget(L, lua_upvalueindex(1));
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushstring(L, names[i]);
        lua_rawget(L, globals);
        int allowed = lua_isnil(L, -1) || lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        if (!allowed) return luaL_error(L, "global already exists: %s", names[i]);
    }
    size_t count = 0;
    while (names[count]) ++count;
    unsigned char *inserted = calloc(count ? count : 1, 1);
    if (!inserted) return luaL_error(L, "cannot prepare global import");
    ImportState state = {names, inserted};
    lua_pushcfunction(L, install_globals);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, globals);
    lua_pushlightuserdata(L, &state);
    int status = lua_pcall(L, 3, 0, 0);
    if (status != LUA_OK) {
        for (size_t i = 0; names[i]; ++i) {
            if (!inserted[i]) continue;
            lua_pushstring(L, names[i]);
            lua_pushnil(L);
            lua_rawset(L, globals);
        }
        free(inserted);
        return lua_error(L);
    }
    free(inserted);
    return 0;
}
