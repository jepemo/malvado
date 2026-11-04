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


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include "lua.h"
#include "lauxlib.h"
#include "malvado/malvado.h"
#include "malvado/platform.h"
#include "runner/runtime.h"

enum Failure { NONE, CONTEXT, LUA_STATE, WINDOW };
static enum Failure failure;
static int contexts_created, contexts_destroyed, states_created, states_closed;
static int windows_opened, windows_closed;

MvContext *test_context_create(double rate, unsigned steps)
{
    if (failure == CONTEXT) return NULL;
    MvContext *ctx = mv_context_create(rate, steps);
    if (ctx) contexts_created++;
    return ctx;
}

void test_context_destroy(MvContext *ctx)
{
    if (ctx) contexts_destroyed++;
    mv_context_destroy(ctx);
}

lua_State *test_lua_newstate(void)
{
    if (failure == LUA_STATE) return NULL;
    lua_State *L = luaL_newstate();
    if (L) states_created++;
    return L;
}

void test_lua_close(lua_State *L)
{
    if (L) states_closed++;
    lua_close(L);
}

int test_window_open(int width, int height, const char *title)
{
    if (failure == WINDOW) return 0;
    int opened = mv_window_open(width, height, title);
    if (opened) windows_opened++;
    return opened;
}

void test_window_close(void)
{
    windows_closed++;
    mv_window_close();
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    char *headless[] = {"malvado-run", "--headless", argv[1]};
    char *windowed[] = {"malvado-run", argv[1]};
    for (int i = CONTEXT; i <= WINDOW; i++) {
        contexts_created = contexts_destroyed = states_created = states_closed = 0;
        windows_opened = windows_closed = 0;
        failure = (enum Failure)i;
        assert(malvado_run(i == WINDOW ? 2 : 3, i == WINDOW ? windowed : headless) == 1);
        assert(contexts_created == (i == CONTEXT ? 0 : 1));
        assert(states_created == (i == WINDOW ? 1 : 0));
        assert(contexts_created == contexts_destroyed);
        assert(states_created == states_closed);
        assert(windows_opened == 0 && windows_closed == 0);
        assert(malvado_debug_context() == NULL);
        failure = NONE;
        assert(malvado_run(3, headless) == 0);
        assert(contexts_created == contexts_destroyed);
        assert(states_created == states_closed);
    }
    return 0;
}
