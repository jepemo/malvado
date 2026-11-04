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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "malvado/platform.h"
#include "runner/runtime.h"

static size_t allocation_count, fail_at;
static int armed, succeeded, fail_native;

MvResult test_process_spawn_related(MvContext *ctx, MvProcessId creator, MvProcessId owner,
                                    int64_t priority, MvProcessStep step,
                                    MvProcessDestroy destroy, void *user, MvProcessId *id)
{
    if (fail_native) return MV_OUT_OF_MEMORY;
    return mv_process_spawn_related(ctx, creator, owner, priority, step, destroy, user, id);
}

static void *
allocate(void *user, void *ptr, size_t old_size, size_t new_size)
{
    (void)user;
    if (!new_size) {
        free(ptr);
        return NULL;
    }
    if (armed && (!ptr || new_size > old_size) && allocation_count++ >= fail_at)
        return NULL;
    return realloc(ptr, new_size);
}

static int
arm(lua_State *L)
{
    (void)L;
    allocation_count = 0;
    armed = 1;
    return 0;
}

static int
disarm(lua_State *L)
{
    (void)L;
    armed = 0;
    fail_native = 0;
    return 0;
}

static int
report(lua_State *L)
{
    succeeded = lua_toboolean(L, 1);
    return 0;
}

lua_State *test_lua_newstate(void) { return lua_newstate(allocate, NULL); }
void test_lua_openlibs(lua_State *L)
{
    luaL_openlibs(L);
    lua_pushcfunction(L, arm); lua_setglobal(L, "test_arm");
    lua_pushcfunction(L, disarm); lua_setglobal(L, "test_disarm");
    lua_pushcfunction(L, report); lua_setglobal(L, "test_report");
}
void test_lua_close(lua_State *L) { lua_close(L); }
MvContext *test_context_create(double rate, unsigned steps) { return mv_context_create(rate, steps); }
void test_context_destroy(MvContext *ctx) { mv_context_destroy(ctx); }
int test_window_open(int w, int h, const char *title) { return mv_window_open(w, h, title); }
void test_window_close(void) { mv_window_close(); }

int
main(int argc, char **argv)
{
    assert(argc == 2);
    char *args[] = {"malvado-run", "--headless", argv[1]};
    for (fail_at = 0; fail_at < 256; fail_at++) {
        armed = succeeded = 0;
        assert(malvado_run(3, args) == 0);
        assert(!armed);
        if (succeeded) break;
    }
    assert(fail_at > 0 && fail_at < 256);
    printf("tested %zu Lua allocation failures\n", fail_at);
    fail_at = SIZE_MAX;
    fail_native = 1;
    succeeded = 1;
    assert(malvado_run(3, args) == 0);
    assert(!succeeded && !fail_native && !armed);
    return 0;
}
