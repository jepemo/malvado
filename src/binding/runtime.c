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

#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "malvado/platform.h"
#ifdef MALVADO_BUILD_ASSETS
#include "malvado/atlas.h"
#endif
#include "core/clock.h"
#include "core/schedule.h"
#include "malvado/malvado.h"
#include "runner/runtime.h"

typedef enum {
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_FROZEN,
    PROCESS_KILLED
} ProcessState;

typedef struct {
    lua_Integer id;
    lua_Integer type_id;
    lua_State *thread;
    int thread_ref;
    int self_ref;
    int draw_ref;
    lua_Integer parent_id;
    lua_Integer priority;
    ProcessState state;
    int first;
    int alive;
} Process;
typedef struct {
    lua_State *L;
    MvContext *context;
    Process *items;
    size_t count, capacity;
    int active, headless, started, failed, window;
    int in_tick, paused, step_requested;
    lua_State *active_thread;
    lua_Integer active_id;
    lua_Integer next_process_id;
    lua_Integer next_type_id;
    lua_Integer tick_number;
    lua_Integer next_text_id;
    MvColor background;
} Engine;
static Engine engine;
static MalvadoDebugHook debug_hook;
static void *debug_user;
static MalvadoDebugHook debug_frame_hook;
static void *debug_frame_user;
static MvStepResult lua_process_step(MvContext *ctx, MvProcessId id, void *user);
static void lua_process_destroy(void *user);

typedef struct { lua_Integer id; } LuaProcessHandle;
typedef struct { MvImage *image; } LuaImage;
#ifdef MALVADO_BUILD_ASSETS
typedef struct { MvAtlas *atlas; } LuaAtlas;
#endif

MvContext *
malvado_debug_context(void)
{
    return engine.context;
}

static MvSignal
core_signal(ProcessState state)
{
    switch (state) {
    case PROCESS_RUNNING: return MV_SIGNAL_WAKEUP;
    case PROCESS_SLEEPING: return MV_SIGNAL_SLEEP;
    case PROCESS_FROZEN: return MV_SIGNAL_FREEZE;
    case PROCESS_KILLED: return MV_SIGNAL_KILL;
    }
    return MV_SIGNAL_KILL;
}

void
malvado_debug_set_hook(MalvadoDebugHook hook, void *user)
{
    debug_hook = hook;
    debug_user = user;
    if (!hook) {
        engine.paused = 0;
        engine.step_requested = 0;
    }
}


void
malvado_debug_set_frame_hook(MalvadoDebugHook hook, void *user)
{
    debug_frame_hook = hook;
    debug_frame_user = user;
}

/* Slots are compacted at tick boundaries; public IDs are never reused. */
static Process *
find_process_slot(lua_Integer id)
{
    if (id <= 0)
        return NULL;
    for (size_t i = 0; i < engine.count; i++) {
        if (engine.items[i].alive && engine.items[i].id == id)
            return &engine.items[i];
    }
    return NULL;
}

static Process *
find_process(lua_Integer id)
{
    Process *process = find_process_slot(id);
    if (!process || process->state == PROCESS_KILLED)
        return NULL;
    return process;
}

static void
sync_processes(void)
{
    for (size_t i = 0; i < engine.count; ++i) {
        Process *p = &engine.items[i];
        MvProcessStatus status;
        int64_t priority;
        if (!p->alive) continue;
        if (mv_process_status(engine.context, (MvProcessId)p->id, &status) != MV_OK)
            p->state = PROCESS_KILLED;
        else {
            p->state = status == MV_PROCESS_RUNNING ? PROCESS_RUNNING :
                       status == MV_PROCESS_SLEEPING ? PROCESS_SLEEPING : PROCESS_FROZEN;
            if (mv_process_priority(engine.context, (MvProcessId)p->id, &priority) == MV_OK)
                p->priority = priority;
        }
    }
}

int
malvado_debug_snapshot(MalvadoProcessInfo *out, size_t capacity,
                       size_t *total, int64_t *tick)
{
    if (!engine.L || engine.in_tick || !total || !tick || (capacity && !out))
        return 0;
    sync_processes();
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo)) return 0;
    MvProcessInfo *items = count ? malloc(count * sizeof(*items)) : NULL;
    if (count && !items) return 0;
    if (mv_process_snapshot(engine.context, items, count, total) != MV_OK) {
        free(items);
        return 0;
    }
    static const char *names[] = {"running", "sleeping", "frozen"};
    for (size_t i = 0; i < count && i < capacity; ++i) {
        out[i] = (MalvadoProcessInfo){
            (int64_t)items[i].id, (int64_t)items[i].parent,
            (int64_t)items[i].type, items[i].priority,
            items[i].messages, names[items[i].status]
        };
    }
    free(items);
    *tick = engine.tick_number;
    return 1;
}

int
malvado_debug_signal(int64_t id, const char *signal)
{
    if (!engine.L || engine.in_tick || !signal)
        return 0;
    ProcessState state;
    if (strcmp(signal, "sleep") == 0)
        state = PROCESS_SLEEPING;
    else if (strcmp(signal, "freeze") == 0)
        state = PROCESS_FROZEN;
    else if (strcmp(signal, "wakeup") == 0)
        state = PROCESS_RUNNING;
    else if (strcmp(signal, "kill") == 0)
        state = PROCESS_KILLED;
    else
        return 0;
    int ok = mv_process_signal(engine.context, (MvProcessId)id, core_signal(state), 0) == MV_OK;
    sync_processes();
    return ok;
}

int
malvado_debug_pause(int paused)
{
    if (!engine.L || engine.in_tick || !debug_hook)
        return 0;
    engine.paused = paused != 0;
    engine.step_requested = 0;
    return 1;
}

int
malvado_debug_step(void)
{
    if (!engine.L || engine.in_tick || !engine.paused || !debug_hook)
        return 0;
    engine.step_requested = 1;
    return 1;
}

typedef struct {
    lua_State *state;
    int reference;
} LuaMessage;

static void
release_lua_message(void *data)
{
    LuaMessage *value = data;
    luaL_unref(value->state, LUA_REGISTRYINDEX, value->reference);
    free(value);
}

static int
receive_message(lua_State *L)
{
    lua_Integer id = lua_tointeger(L, lua_upvalueindex(1));
    MvMessage message;
    MvResult result = mv_process_receive(engine.context, (MvProcessId)id, &message);
    if (result == MV_QUEUE_EMPTY) {
        lua_pushnil(L);
        return 1;
    }
    if (result != MV_OK)
        return luaL_error(L, "unknown process id");
    switch (message.type) {
    case MV_MESSAGE_BOOLEAN: lua_pushboolean(L, message.value.boolean); break;
    case MV_MESSAGE_INTEGER: lua_pushinteger(L, message.value.integer); break;
    case MV_MESSAGE_NUMBER: lua_pushnumber(L, message.value.number); break;
    case MV_MESSAGE_BYTES:
        lua_pushlstring(L, message.value.bytes.data ? message.value.bytes.data : "", message.value.bytes.size);
        break;
    case MV_MESSAGE_OPAQUE:
        if (message.value.opaque.destroy != release_lua_message) {
            mv_message_release(&message);
            return luaL_error(L, "unsupported native message");
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, ((LuaMessage *)message.value.opaque.data)->reference);
        break;
    }
    mv_message_release(&message);
    return 1;
}

static int
send_message(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    luaL_checkany(L, 2);
    if (lua_isnil(L, 2))
        return luaL_error(L, "message must not be nil");
    MvMessage message = {0};
    switch (lua_type(L, 2)) {
    case LUA_TBOOLEAN:
        message.type = MV_MESSAGE_BOOLEAN;
        message.value.boolean = lua_toboolean(L, 2);
        break;
    case LUA_TNUMBER:
        if (lua_isinteger(L, 2)) {
            message.type = MV_MESSAGE_INTEGER;
            message.value.integer = lua_tointeger(L, 2);
        } else {
            message.type = MV_MESSAGE_NUMBER;
            message.value.number = lua_tonumber(L, 2);
        }
        break;
    case LUA_TSTRING: {
        size_t size;
        const char *value = lua_tolstring(L, 2, &size);
        message.type = MV_MESSAGE_BYTES;
        message.value.bytes.data = (void *)value;
        message.value.bytes.size = size;
        break;
    }
    default: {
        if (!find_process(id))
            return luaL_error(L, "non-primitive messages require a Lua recipient");
        LuaMessage *value = malloc(sizeof(*value));
        if (!value) return luaL_error(L, "out of memory creating message");
        lua_pushvalue(L, 2);
        value->reference = luaL_ref(L, LUA_REGISTRYINDEX);
        value->state = engine.L;
        message.type = MV_MESSAGE_OPAQUE;
        message.value.opaque.data = value;
        message.value.opaque.destroy = release_lua_message;
        break;
    }
    }
    MvResult result = mv_process_send(engine.context, (MvProcessId)id, &message);
    if (result != MV_OK) {
        if (message.type == MV_MESSAGE_OPAQUE) mv_message_release(&message);
        if (result == MV_QUEUE_FULL) return luaL_error(L, "message queue is full");
        if (result == MV_NOT_FOUND) return luaL_error(L, "unknown process id");
        return luaL_error(L, "failed to send message");
    }
    return 0;
}

static ProcessState
parse_signal(lua_State *L, int argument)
{
    const char *signal = luaL_checkstring(L, argument);
    if (strcmp(signal, "kill") == 0)
        return PROCESS_KILLED;
    if (strcmp(signal, "sleep") == 0)
        return PROCESS_SLEEPING;
    if (strcmp(signal, "freeze") == 0)
        return PROCESS_FROZEN;
    if (strcmp(signal, "wakeup") == 0)
        return PROCESS_RUNNING;
    luaL_error(L, "unknown signal: %s", signal);
    return PROCESS_KILLED; /* luaL_error does not return. */
}

static int
signal_process(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    ProcessState state = parse_signal(L, 2);
    if (mv_process_signal(engine.context, (MvProcessId)id, core_signal(state), 0) != MV_OK)
        return luaL_error(L, "unknown process id");
    sync_processes();
    return 0;
}

static int
signal_tree(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    ProcessState state = parse_signal(L, 2);
    if (mv_process_signal(engine.context, (MvProcessId)id, core_signal(state), 1) != MV_OK)
        return luaL_error(L, "unknown process id");
    sync_processes();
    return 0;
}

static int
kill_tree(lua_State *L)
{
    lua_settop(L, 1);
    lua_pushliteral(L, "kill");
    return signal_tree(L);
}

static int
set_priority(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_Integer priority = luaL_checkinteger(L, 2);
    if (mv_process_set_priority(engine.context, (MvProcessId)id, priority) != MV_OK)
        return luaL_error(L, "unknown process id");
    sync_processes();
    return 0;
}

static int
get_priority(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    int64_t priority;
    if (mv_process_priority(engine.context, (MvProcessId)id, &priority) != MV_OK)
        return luaL_error(L, "unknown process id");
    lua_pushinteger(L, priority);
    return 1;
}

static int
kill_process(lua_State *L)
{
    lua_settop(L, 1);
    lua_pushliteral(L, "kill");
    return signal_process(L);
}

static int
process_status(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK) {
        lua_pushnil(L);
        return 1;
    }
    const char *states[] = {"running", "sleeping", "frozen"};
    lua_pushstring(L, states[status]);
    return 1;
}

typedef struct { lua_Integer id, parent; } LuaSelf;

static int
self_index(lua_State *L)
{
    LuaSelf *self = luaL_checkudata(L, 1, "malvado.process");
    const char *key = lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : NULL;
    if (key && strcmp(key, "id") == 0) {
        lua_pushinteger(L, self->id);
        return 1;
    }
    if (key && strcmp(key, "parent") == 0) {
        if (self->parent) lua_pushinteger(L, self->parent);
        else lua_pushnil(L);
        return 1;
    }
    if (key && strcmp(key, "priority") == 0) {
        int64_t priority;
        if (mv_process_priority(engine.context, (MvProcessId)self->id, &priority) != MV_OK)
            return luaL_error(L, "process is no longer alive");
        lua_pushinteger(L, priority);
        return 1;
    }
    if (key && strcmp(key, "status") == 0) {
        MvProcessStatus status;
        static const char *names[] = {"running", "sleeping", "frozen"};
        if (mv_process_status(engine.context, (MvProcessId)self->id, &status) != MV_OK)
            lua_pushnil(L);
        else
            lua_pushstring(L, names[status]);
        return 1;
    }
    if (key && strcmp(key, "recv") == 0) {
        lua_pushinteger(L, self->id);
        lua_pushcclosure(L, receive_message, 1);
        return 1;
    }
    lua_getiuservalue(L, 1, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int
self_newindex(lua_State *L)
{
    luaL_checkudata(L, 1, "malvado.process");
    const char *key = lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : NULL;
    if (key && (!strcmp(key, "id") || !strcmp(key, "parent") ||
                !strcmp(key, "priority") || !strcmp(key, "status") || !strcmp(key, "recv")))
        return luaL_error(L, "process field is read-only");
    lua_getiuservalue(L, 1, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

static int
create_process(lua_State *L)
{
    if (!lua_isnoneornil(L, 1))
        luaL_checktype(L, 1, LUA_TTABLE);
    if (engine.next_process_id == LUA_MAXINTEGER)
        return luaL_error(L, "process ID space exhausted");
    if (engine.count == engine.capacity) {
        if (engine.capacity > SIZE_MAX / 2 / sizeof(*engine.items))
            return luaL_error(L, "too many processes");
        size_t n = engine.capacity ? engine.capacity * 2 : 16;
        Process *p = realloc(engine.items, n * sizeof(*p));
        if (!p)
            return luaL_error(L, "out of memory creating process");
        engine.items = p;
        engine.capacity = n;
    }
    LuaProcessHandle *handle = malloc(sizeof(*handle));
    if (!handle)
        return luaL_error(L, "out of memory creating process");
    MvProcessId core_id;
    MvProcessId parent = (MvProcessId)engine.active_id;
    MvProcessStatus parent_status;
    if (parent && mv_process_status(engine.context, parent, &parent_status) != MV_OK)
        parent = 0;
    MvResult spawn_result = mv_process_spawn(engine.context, parent, 0,
                                             lua_process_step, lua_process_destroy,
                                             handle, &core_id);
    if (spawn_result != MV_OK) {
        free(handle);
        return luaL_error(L, "failed to create process");
    }
    handle->id = (lua_Integer)core_id;
    (void)mv_process_set_type(engine.context, core_id, (uint64_t)lua_tointeger(L, lua_upvalueindex(2)));
    engine.next_process_id = handle->id;
    lua_newtable(L);
    if (!lua_isnoneornil(L, 1)) {
        lua_pushnil(L);
        while (lua_next(L, 1)) {
            lua_pushvalue(L, -2);
            lua_pushvalue(L, -2);
            lua_settable(L, -5);
            lua_pop(L, 1);
        }
    }
    lua_Integer id = handle->id;
    LuaSelf *self = lua_newuserdatauv(L, sizeof(*self), 1);
    *self = (LuaSelf){id, engine.active_id};
    lua_pushvalue(L, -2);
    lua_setiuservalue(L, -2, 1);
    lua_remove(L, -2);
    luaL_getmetatable(L, "malvado.process");
    lua_setmetatable(L, -2);
    int self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_State *co = lua_newthread(L);
    int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_xmove(L, co, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    lua_xmove(L, co, 1);
    lua_newtable(L);
    int draw_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    /* Lua allocations may run finalizers that create more processes. */
    if (engine.count == engine.capacity) {
        if (engine.capacity > SIZE_MAX / 2 / sizeof(*engine.items))
            return luaL_error(L, "too many processes");
        size_t n = engine.capacity ? engine.capacity * 2 : 16;
        Process *items = realloc(engine.items, n * sizeof(*items));
        if (!items)
            return luaL_error(L, "out of memory creating process");
        engine.items = items;
        engine.capacity = n;
    }
    engine.items[engine.count++] = (Process) {
        .id = id,
        .type_id = lua_tointeger(L, lua_upvalueindex(2)),
        .thread = co,
        .thread_ref = thread_ref,
        .self_ref = self_ref,
        .draw_ref = draw_ref,
        .parent_id = engine.active_id,
        .state = PROCESS_RUNNING,
        .first = 1,
        .alive = 1
    };
    lua_pushinteger(L, id);
    return 1;
}

static int
define_process(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (engine.next_type_id == LUA_MAXINTEGER)
        return luaL_error(L, "process type ID space exhausted");
    lua_pushvalue(L, 1);
    lua_pushinteger(L, ++engine.next_type_id);
    lua_pushcclosure(L, create_process, 2);
    return 1;
}

/* Results are copies, never references to mutable scheduler storage. */
static const char *
state_name(ProcessState state)
{
    static const char *names[] = {"running", "sleeping", "frozen", "killed"};
    return names[state];
}

static int
find_by_type(lua_State *L)
{
    if (!lua_iscfunction(L, 1) || lua_tocfunction(L, 1) != create_process)
        return luaL_error(L, "expected a process constructor");
    if (!lua_getupvalue(L, 1, 2))
        return luaL_error(L, "invalid process constructor");
    lua_Integer type_id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo))
        return luaL_error(L, "too many processes");
    MvProcessInfo *items = lua_newuserdatauv(L, count * sizeof(*items), 0);
    size_t total;
    if (mv_process_snapshot(engine.context, items, count, &total) != MV_OK)
        return luaL_error(L, "failed to inspect processes");
    lua_newtable(L);
    lua_Integer result = 0;
    for (size_t i = 0; i < count && i < total; i++) {
        if (items[i].type == (uint64_t)type_id && items[i].id <= LUA_MAXINTEGER) {
            lua_pushinteger(L, (lua_Integer)items[i].id);
            lua_seti(L, -2, ++result);
        }
    }
    return 1;
}

static int
snapshot(lua_State *L)
{
    sync_processes();
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo))
        return luaL_error(L, "too many processes");
    MvProcessInfo *items = lua_newuserdatauv(L, count * sizeof(*items), 0);
    size_t total;
    if (mv_process_snapshot(engine.context, items, count, &total) != MV_OK)
        return luaL_error(L, "failed to inspect processes");
    lua_newtable(L);
    lua_pushinteger(L, engine.tick_number);
    lua_setfield(L, -2, "tick");
    lua_newtable(L);
    lua_Integer result = 0;
    for (size_t i = 0; i < count && i < total; i++) {
        MvProcessInfo *p = &items[i];
        Process *lua_process = find_process_slot((lua_Integer)p->id);
        int draw_ref = lua_process ? lua_process->draw_ref : LUA_NOREF;
        lua_createtable(L, 0, 8);
        lua_pushinteger(L, (lua_Integer)p->id); lua_setfield(L, -2, "id");
        if (p->parent) {
            lua_pushinteger(L, (lua_Integer)p->parent); lua_setfield(L, -2, "parent");
        }
        lua_pushinteger(L, (lua_Integer)p->type); lua_setfield(L, -2, "type");
        lua_pushinteger(L, p->priority); lua_setfield(L, -2, "priority");
        lua_pushstring(L, state_name((ProcessState)p->status)); lua_setfield(L, -2, "status");
        lua_pushinteger(L, (lua_Integer)p->messages); lua_setfield(L, -2, "messages");
        lua_rawgeti(L, LUA_REGISTRYINDEX, draw_ref);
        lua_Integer draw_count = (lua_Integer)lua_rawlen(L, -1);
        lua_pop(L, 1);
        lua_pushinteger(L, draw_count); lua_setfield(L, -2, "draw_commands");
        lua_pushboolean(L, p->status != MV_PROCESS_SLEEPING); lua_setfield(L, -2, "visible");
        lua_seti(L, -2, ++result);
    }
    lua_setfield(L, -2, "processes");
    return 1;
}

static int
frame(lua_State *L)
{
    if (!engine.active || L != engine.active_thread || !lua_isyieldable(L))
        return luaL_error(L, "frame() must be called inside a process");
    return lua_yield(L, 0);
}

static int
clear_screen(lua_State *L)
{
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        return luaL_error(L, "color must be in 0..255");
    engine.background = (MvColor) {
        r, g, b, 255
    };
    return 0;
}

static int
key(lua_State *L)
{
    size_t length;
    const char *s = luaL_checklstring(L, 1, &length);
    MvKey parsed;
    if (strlen(s) != length || !mv_key_from_name(s, &parsed))
        return luaL_error(L, "unsupported key: %s", s);
    lua_pushboolean(L, !engine.headless && mv_key_down(parsed));
    return 1;
}

static int
mouse_position(lua_State *L)
{
    int x = 0, y = 0;
    if (!engine.headless)
        (void)mv_mouse_position(&x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int
mouse_button(lua_State *L)
{
    size_t length;
    const char *name = luaL_checklstring(L, 1, &length);
    MvMouseButton button;
    if (strlen(name) != length || !mv_mouse_button_from_name(name, &button))
        return luaL_error(L, "unsupported mouse button: %s", name);
    lua_pushboolean(L, !engine.headless && mv_mouse_down(button));
    return 1;
}

static int
image_gc(lua_State *L)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    mv_image_destroy(image->image);
    image->image = NULL;
    return 0;
}

static int
load_image(lua_State *L)
{
    size_t length;
    const char *path = luaL_checklstring(L, 1, &length);
    if (strlen(path) != length)
        return luaL_error(L, "image path contains a zero byte");
    if (!engine.window)
        return luaL_error(L, "load_image requires an open window");
    LuaImage *image = lua_newuserdatauv(L, sizeof(*image), 0);
    image->image = NULL;
    luaL_setmetatable(L, "malvado.image");
    image->image = mv_image_load(path);
    if (!image->image)
        return luaL_error(L, "could not load PNG image: %s", path);
    return 1;
}

static int
image_size(lua_State *L)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    lua_pushinteger(L, mv_image_width(image->image));
    lua_pushinteger(L, mv_image_height(image->image));
    return 2;
}

static int
draw_image(lua_State *L)
{
    luaL_checkudata(L, 1, "malvado.image");
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);
    if (!engine.active || L != engine.active_thread)
        return luaL_error(L, "draw_image must be called inside a process");
    Process *process = find_process(engine.active_id);
    if (!process)
        return luaL_error(L, "draw_image must be called inside a live process");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
    lua_createtable(L, 3, 0);
    for (int i = 1; i <= 3; ++i) {
        lua_pushvalue(L, i);
        lua_seti(L, -2, i);
    }
    lua_seti(L, -2, lua_rawlen(L, -2) + 1);
    lua_pop(L, 1);
    return 0;
}

static int
region_integer(lua_State *L, int index)
{
    lua_Integer value = luaL_checkinteger(L, index);
    if (value < 0 || value > INT_MAX)
        luaL_argerror(L, index, "expected a non-negative integer within range");
    return (int)value;
}

static float
region_number(lua_State *L, int index, lua_Number fallback)
{
    lua_Number value = index <= 3 ? luaL_checknumber(L, index) : luaL_optnumber(L, index, fallback);
    if (!isfinite(value) || value < -FLT_MAX || value > FLT_MAX)
        luaL_argerror(L, index, "expected a finite number within range");
    return (float)value;
}

static int
draw_image_region(lua_State *L)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    MvImageTransform transform = {
        region_number(L, 2, 0), region_number(L, 3, 0),
        region_number(L, 8, 0), region_number(L, 9, 0),
        region_number(L, 10, 1), region_number(L, 11, 0)
    };
    MvImageRegion region = {
        region_integer(L, 4), region_integer(L, 5),
        region_integer(L, 6), region_integer(L, 7)
    };
    if (!mv_image_region_valid(image->image, region, transform))
        return luaL_error(L, "invalid image region or transform");
    if (!engine.active || L != engine.active_thread)
        return luaL_error(L, "draw_image_region must be called inside a process");
    Process *process = find_process(engine.active_id);
    if (!process)
        return luaL_error(L, "draw_image_region must be called inside a live process");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
    lua_createtable(L, 11, 0);
    lua_pushvalue(L, 1);
    lua_seti(L, -2, 1);
    const lua_Number values[] = {
        transform.x, transform.y, region.x, region.y, region.width, region.height,
        transform.pivot_x, transform.pivot_y, transform.scale, transform.rotation
    };
    for (int i = 0; i < 10; ++i) {
        lua_pushnumber(L, values[i]);
        lua_seti(L, -2, i + 2);
    }
    lua_seti(L, -2, lua_rawlen(L, -2) + 1);
    lua_pop(L, 1);
    return 0;
}

#ifdef MALVADO_BUILD_ASSETS
static int
atlas_gc(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    mv_atlas_destroy(atlas->atlas);
    atlas->atlas = NULL;
    return 0;
}

static int
load_atlas(lua_State *L)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    size_t length;
    const char *json = luaL_checklstring(L, 2, &length);
    LuaAtlas *atlas = lua_newuserdatauv(L, sizeof(*atlas), 1);
    atlas->atlas = NULL;
    luaL_setmetatable(L, "malvado.atlas");
    const char *error = NULL;
    atlas->atlas = mv_atlas_parse(json, length, mv_image_width(image->image),
                                  mv_image_height(image->image), &error);
    if (!atlas->atlas)
        return luaL_error(L, "%s", error);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    return 1;
}

static int
draw_sprite(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    lua_Integer id = luaL_checkinteger(L, 2);
    lua_Number x = luaL_checknumber(L, 3);
    lua_Number y = luaL_checknumber(L, 4);
    lua_Number scale = luaL_optnumber(L, 5, 1);
    lua_Number rotation = luaL_optnumber(L, 6, 0);
    MvAtlasSprite sprite;
    if (id <= 0 || id > INT_MAX || !mv_atlas_find(atlas->atlas, (int)id, &sprite))
        return luaL_error(L, "unknown sprite id");
    lua_getiuservalue(L, 1, 1);
    lua_replace(L, 1);
    lua_settop(L, 1);
    const lua_Number values[] = {
        x, y, sprite.region.x, sprite.region.y, sprite.region.width, sprite.region.height,
        sprite.pivot_x, sprite.pivot_y, scale, rotation
    };
    for (int i = 0; i < 10; ++i)
        lua_pushnumber(L, values[i]);
    return draw_image_region(L);
}
#endif

static int
text(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int size = luaL_optinteger(L, 4, 20);
    if (!engine.active)
        return luaL_error(L, "draw_text must be called inside a process");

    /* Draw commands live until the next simulation tick. */
    Process *process = find_process(engine.active_id);
    if (!process || L != engine.active_thread)
        return luaL_error(L, "draw_text must be called inside a live process");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
    lua_createtable(L, 4, 0);
    lua_pushstring(L, s);
    lua_seti(L, -2, 1);
    lua_pushinteger(L, x);
    lua_seti(L, -2, 2);
    lua_pushinteger(L, y);
    lua_seti(L, -2, 3);
    lua_pushinteger(L, size);
    lua_seti(L, -2, 4);
    lua_seti(L, -2, lua_rawlen(L, -2) + 1);
    lua_pop(L, 1);
    return 0;
}
/* The Lua state owns text objects and fonts until they are deleted or closed. */
static int
font(lua_State *L)
{
    lua_Integer size = luaL_checkinteger(L, 1);
    if (size < 1 || size > 1024)
        return luaL_error(L, "font size must be in 1..1024");
    lua_Integer channels[4];
    for (int i = 0; i < 4; ++i) {
        channels[i] = luaL_optinteger(L, i + 2, 255);
        if (channels[i] < 0 || channels[i] > 255)
            return luaL_error(L, "color must be in 0..255");
    }
    lua_newtable(L);
    lua_pushinteger(L, size);
    lua_setfield(L, -2, "size");
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, channels[i]);
        lua_seti(L, -2, i + 1);
    }
    return 1;
}

static int
write_text(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "size");
    lua_Integer size = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (size < 1 || size > 1024)
        return luaL_error(L, "invalid font");
    lua_Integer x = luaL_checkinteger(L, 2);
    lua_Integer y = luaL_checkinteger(L, 3);
    const char *text = luaL_checkstring(L, 4);
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_createtable(L, 0, 5);
    lua_pushstring(L, text);
    lua_setfield(L, -2, "text");
    lua_pushinteger(L, x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, y);
    lua_setfield(L, -2, "y");
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "font");
    lua_Integer id = ++engine.next_text_id;
    lua_seti(L, -2, id);
    lua_pop(L, 1);
    lua_pushinteger(L, id);
    return 1;
}

static int
delete_text(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_geti(L, -1, id);
    if (lua_isnil(L, -1))
        return luaL_error(L, "unknown text id");
    lua_pop(L, 1);
    lua_pushnil(L);
    lua_seti(L, -2, id);
    return 0;
}

static int
change_text(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    const char *text = luaL_checkstring(L, 2);
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_geti(L, -1, id);
    if (lua_isnil(L, -1))
        return luaL_error(L, "unknown text id");
    lua_pushstring(L, text);
    lua_setfield(L, -2, "text");
    return 0;
}

static void
render_texts(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    for (lua_Integer id = 1; id <= engine.next_text_id; ++id) {
        lua_geti(L, -1, id);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_getfield(L, -1, "text");
        const char *text = lua_tostring(L, -1);
        lua_getfield(L, -2, "x");
        int x = (int)lua_tointeger(L, -1);
        lua_getfield(L, -3, "y");
        int y = (int)lua_tointeger(L, -1);
        lua_getfield(L, -4, "font");
        lua_getfield(L, -1, "size");
        int size = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        MvColor c = {255, 255, 255, 255};
        unsigned char *channels[] = {&c.r, &c.g, &c.b, &c.a};
        for (int i = 0; i < 4; ++i) {
            lua_geti(L, -1, i + 1);
            *channels[i] = (unsigned char)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        mv_window_draw_text(text, x, y, size, c);
        lua_pop(L, 5);
    }
    lua_pop(L, 1);
}

static void
cleanup(Process *p)
{
    if (!p->alive)
        return;
    /* Closing Lua variables may run callbacks which create more processes. */
    lua_State *thread = p->thread;
    int thread_ref = p->thread_ref;
    int self_ref = p->self_ref;
    int draw_ref = p->draw_ref;
    p->alive = 0;
    int status = lua_resetthread(thread);
    if (status != LUA_OK) {
        fprintf(stderr, "Error closing process: %s\n", lua_tostring(thread, -1));
        engine.failed = 1;
    }
    luaL_unref(engine.L, LUA_REGISTRYINDEX, thread_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, self_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, draw_ref);
}

static int
lua_process_step_protected(lua_State *L)
{
    MvProcessId id = (MvProcessId)lua_tointeger(L, 1);
    sync_processes();
    Process *process = find_process((lua_Integer)id);
    if (!process) {
        lua_pushinteger(L, MV_STEP_ERROR);
        return 1;
    }
    lua_State *co = process->thread;
    int draw_ref = process->draw_ref;
    int first = process->first;
    process->first = 0;
    lua_newtable(L);
    lua_rawseti(L, LUA_REGISTRYINDEX, draw_ref);
    engine.active = 1;
    engine.active_thread = co;
    engine.active_id = (lua_Integer)id;
    int results = 0;
    int status = lua_resume(co, L, first ? 1 : 0, &results);
    engine.active = 0;
    engine.active_thread = NULL;
    engine.active_id = 0;
    if (status != LUA_YIELD && status != LUA_OK) {
        luaL_traceback(L, co, lua_tostring(co, -1), 1);
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        engine.failed = 1;
        lua_pushinteger(L, MV_STEP_ERROR);
        return 1;
    }
    if (results)
        lua_pop(co, results);
    lua_pushinteger(L, status == LUA_OK ? MV_STEP_DONE : MV_STEP_YIELD);
    return 1;
}

static MvStepResult
lua_process_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)user;
    lua_State *L = engine.L;
    if (!lua_checkstack(L, 2)) {
        fprintf(stderr, "Unable to prepare Lua process callback\n");
        engine.failed = 1;
        return MV_STEP_ERROR;
    }
    /* Keep Lua allocation errors inside a protected call, not across the core. */
    lua_pushcfunction(L, lua_process_step_protected);
    lua_pushinteger(L, (lua_Integer)id);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        const char *error = lua_tostring(L, -1);
        fprintf(stderr, "Lua process callback failed: %s\n", error ? error : "unknown error");
        lua_pop(L, 1);
        engine.active = 0;
        engine.active_thread = NULL;
        engine.active_id = 0;
        engine.failed = 1;
        return MV_STEP_ERROR;
    }
    MvStepResult result = (MvStepResult)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return result;
}

static void
lua_process_destroy(void *user)
{
    LuaProcessHandle *handle = user;
    Process *process = find_process_slot(handle->id);
    if (process)
        cleanup(process);
    free(handle);
}

static int
tick(void)
{
    engine.in_tick = 1;
    engine.tick_number++;
    MvResult result = mv_process_tick(engine.context);
    sync_processes();
    if (result != MV_OK) {
        if (!engine.failed)
            fprintf(stderr, "Error scheduling processes: %d\n", (int)result);
        engine.failed = 1;
    }
    size_t write = 0;
    for (size_t i = 0; i < engine.count; i++) {
        if (!engine.items[i].alive)
            continue;
        MvProcessStatus parent_status;
        if (engine.items[i].parent_id &&
            mv_process_status(engine.context, (MvProcessId)engine.items[i].parent_id, &parent_status) != MV_OK)
            engine.items[i].parent_id = 0;
        if (write != i)
            engine.items[write] = engine.items[i];
        write++;
    }
    engine.count = write;
    engine.in_tick = 0;
    return mv_process_count(engine.context) != 0;
}

static void
render_image_region(lua_State *L, int command, LuaImage *image)
{
    lua_Number values[10];
    for (int i = 0; i < 10; ++i) {
        lua_geti(L, command, i + 2);
        values[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    MvImageRegion region = {(int)values[2], (int)values[3], (int)values[4], (int)values[5]};
    MvImageTransform transform = {(float)values[0], (float)values[1],
                                  (float)values[6], (float)values[7],
                                  (float)values[8], (float)values[9]};
    (void)mv_image_draw_region(image->image, region, transform);
}

static void
render(void)
{
    lua_State *L = engine.L;
    mv_window_begin(engine.background);
    for (size_t p = 0; p < engine.count; p++) {
        Process *process = &engine.items[p];
        if (!process->alive || process->state == PROCESS_SLEEPING || process->state == PROCESS_KILLED)
            continue;
        lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
        for (lua_Integer i = 1; i <= (lua_Integer)lua_rawlen(L, -1); i++) {
            lua_geti(L, -1, i);
            lua_geti(L, -1, 1);
            lua_geti(L, -2, 2);
            lua_geti(L, -3, 3);
            LuaImage *image = luaL_testudata(L, -3, "malvado.image");
            int command = lua_absindex(L, -4);
            if (image && lua_rawlen(L, command) == 11) {
                render_image_region(L, command, image);
            } else if (image) {
                mv_image_draw(image->image, (int)lua_tointeger(L, -2), (int)lua_tointeger(L, -1));
            } else {
                const char *value = lua_tostring(L, -3);
                lua_geti(L, -4, 4);
                int size = lua_tointeger(L, -1);
                mv_window_draw_text(value, (int)lua_tointeger(L, -3), (int)lua_tointeger(L, -2),
                                    size, (MvColor){255, 255, 255, 255});
                lua_pop(L, 1);
            }
            lua_pop(L, 4);
        }
        lua_pop(L, 1);
    }
    render_texts(L);
    if (debug_frame_hook) {
        mv_window_flush();
        debug_frame_hook(debug_frame_user);
    }
    mv_window_end();
}

static int
start(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (engine.started)
        return luaL_error(L, "start() can only be called once");
    engine.started = 1;
    int w = 800, h = 600;
    const char *title = "Malvado";
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "width");
        w = luaL_optinteger(L, -1, w);
        lua_pop(L, 1);
        lua_getfield(L, 2, "height");
        h = luaL_optinteger(L, -1, h);
        lua_pop(L, 1);
        lua_getfield(L, 2, "title");
        if (!lua_isnil(L, -1))
            title = luaL_checkstring(L, -1);
    }
    if (w <= 0 || h <= 0)
        return luaL_error(L, "window dimensions must be positive");
    if (!engine.headless) {
        if (!mv_window_open(w, h, title))
            return luaL_error(L, "could not open window");
        engine.window = 1;
    }
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        engine.failed = 1;
        return lua_error(L);
    }
    MalvadoClock clock;
    malvado_clock_init(&clock, 60.0, 5);
    double last_time = engine.headless ? 0 : mv_platform_time();
    int alive = tick();
    if (debug_hook)
        debug_hook(debug_user);
    while (!engine.failed && alive && (engine.headless || !mv_window_should_close())) {
        if (engine.paused) {
            if (!engine.headless)
                render();
            if (debug_hook)
                debug_hook(debug_user);
            if (engine.step_requested) {
                engine.step_requested = 0;
                alive = tick();
                if (debug_hook)
                    debug_hook(debug_user);
            }
            /* Do not accumulate wall time spent paused. */
            if (!engine.headless)
                last_time = mv_platform_time();
            else if (engine.paused && !engine.step_requested)
                mv_platform_wait(0.001);
            continue;
        }
        if (engine.headless) {
            alive = tick();
            if (debug_hook)
                debug_hook(debug_user);
        } else {
            render();
            double now = mv_platform_time();
            unsigned steps = malvado_clock_advance(&clock, now - last_time);
            last_time = now;
            for (unsigned i = 0; i < steps && alive && !engine.failed && !engine.paused; i++) {
                alive = tick();
                if (debug_hook)
                    debug_hook(debug_user);
            }
        }
    }
    if (engine.failed)
        return luaL_error(L, "process failed");
    return 0;
}

static int
open_module(lua_State *L)
{
    static const luaL_Reg functions[] = {
        {"process", define_process},
        {"find", find_by_type},
        {"snapshot", snapshot},
        {"frame", frame},
        {"signal", signal_process},
        {"signal_tree", signal_tree},
        {"set_priority", set_priority},
        {"get_priority", get_priority},
        {"kill", kill_process},
        {"kill_tree", kill_tree},
        {"status", process_status},
        {"send", send_message},
        {"start", start},
        {"clear_screen", clear_screen},
        {"key", key},
        {"mouse_position", mouse_position},
        {"mouse_button", mouse_button},
        {"draw_text", text},
        {"load_image", load_image},
        {"image_size", image_size},
        {"draw_image", draw_image},
        {"draw_image_region", draw_image_region},
#ifdef MALVADO_BUILD_ASSETS
        {"load_atlas", load_atlas},
        {"draw_sprite", draw_sprite},
#endif
        {"font", font},
        {"write", write_text},
        {"change_text", change_text},
        {"delete_text", delete_text},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "malvado.process");
    lua_pushcfunction(L, self_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, self_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushliteral(L, "malvado process");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.image");
    lua_pushcfunction(L, image_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado image");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
#ifdef MALVADO_BUILD_ASSETS
    luaL_newmetatable(L, "malvado.atlas");
    lua_pushcfunction(L, atlas_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado atlas");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
#endif
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    luaL_newlib(L, functions);
    return 1;
}

int
malvado_run(int argc, char **argv)
{
    if (engine.context || engine.L) {
        fprintf(stderr, "A Malvado runner is already active\n");
        return 2;
    }
    memset(&engine, 0, sizeof(engine));
    int index = 1;
    if (argc > 1 && strcmp(argv[1], "--headless") == 0) {
        engine.headless = 1;
        index++;
    }
    if (argc != index + 1) {
        fprintf(stderr, "Usage: malvado-run [--headless] game.lua\n");
        return 2;
    }
    engine.background = (MvColor){0, 0, 0, 255};
    engine.context = mv_context_create(60, 5);
    if (!engine.context)
        return 1;
    engine.L = luaL_newstate();
    if (!engine.L) {
        mv_context_destroy(engine.context);
        engine.context = NULL;
        return 1;
    }
    luaL_openlibs(engine.L);
    luaL_requiref(engine.L, "malvado", open_module, 0);
    lua_pop(engine.L, 1);
    if (luaL_loadfile(engine.L, argv[index]) != LUA_OK || lua_pcall(engine.L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(engine.L, -1));
        engine.failed = 1;
    }
    mv_context_destroy(engine.context);
    engine.context = NULL;
    for (size_t i = 0; i < engine.count; i++)
        cleanup(&engine.items[i]);
    free(engine.items);
    engine.items = NULL;
    engine.count = 0;
    lua_close(engine.L);
    engine.L = NULL;
    if (engine.window)
        mv_window_close();
    return engine.failed ? 1 : 0;
}
