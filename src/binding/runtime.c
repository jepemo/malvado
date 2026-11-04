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
#include <inttypes.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <dirent.h>
#include <errno.h>
#endif
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "malvado/platform.h"
#ifdef MALVADO_BUILD_ASSETS
#include "malvado/atlas.h"
#endif
#include "binding/globals.h"
#include "core/clock.h"
#include "core/process.h"
#include "core/schedule.h"
#include "malvado/malvado.h"
#include "runner/runtime.h"

typedef enum {
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_FROZEN,
    PROCESS_KILLED
} ProcessState;

typedef struct { MvColor target; unsigned ticks; } FadeRequest;

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
    MvCadence cadence;
    lua_Integer last_activation_tick;
    double delta;
    int first;
    int alive;
} Process;
typedef struct {
    lua_State *L;
    MvContext *context;
    Process *items;
    size_t count, capacity;
    int active, headless, started, failed, window, audio;
    int in_tick, paused, step_requested;
    int width, height;
    lua_State *active_thread;
    lua_Integer active_id;
    lua_Integer closing_id;
    lua_Integer next_process_id;
    lua_Integer next_type_id;
    lua_Integer tick_number;
    lua_Integer next_text_id;
    MvColor background;
    MvColor draw_color;
    MvFade fade;
    FadeRequest fade_queue[16];
    unsigned fade_head, fade_count;
} Engine;
static Engine engine;
static MalvadoDebugHook debug_hook;
static void *debug_user;
static MalvadoDebugHook debug_frame_hook;
static void *debug_frame_user;
static MvStepResult lua_process_step(MvContext *ctx, MvProcessId id, void *user);
static void lua_process_destroy(void *user);

typedef struct { lua_Integer id; } LuaProcessHandle;
typedef struct { lua_Integer id, parent; int reason; MvProcessRef reference; } LuaSelf;
typedef struct { MvImage *image; MvResource *resource; } LuaImage;
typedef struct { MvSound *sound; MvResource *resource; int playing, paused; } LuaSound;
typedef struct { MvMusic *music; MvResource *resource; int playing, paused; } LuaMusic;
typedef struct { MvFont *font; MvResource *resource; } LuaFont;
typedef struct { int rows, columns, count, directory; } LuaLegacySheet;
#ifndef _WIN32
typedef struct { char **names; size_t count; } LuaDirectoryEntries;
#endif
#ifdef MALVADO_BUILD_ASSETS
typedef struct { MvAtlas *atlas; MvResource *resource; } LuaAtlas;
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

static lua_Integer
check_process_id(lua_State *L, int index)
{
    LuaSelf *self = luaL_checkudata(L, index, "malvado.process");
    MvProcessId id;
    if (mv_process_ref_resolve(engine.context, self->reference, &id) != MV_OK)
        luaL_error(L, "process handle belongs to another runtime");
    return (lua_Integer)id;
}

static int
send_message(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
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
    size_t length;
    const char *signal = luaL_checklstring(L, argument, &length);
    if (strlen(signal) != length)
        luaL_error(L, "unknown signal");
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

static void
validate_options(lua_State *L, int index, const char *first, const char *second)
{
    index = lua_absindex(L, index);
    luaL_checktype(L, index, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, index)) {
        size_t length = 0;
        const char *name = lua_type(L, -2) == LUA_TSTRING ? lua_tolstring(L, -2, &length) : NULL;
        if (!name || strlen(name) != length ||
            (strcmp(name, first) != 0 && (!second || strcmp(name, second) != 0)))
            luaL_error(L, "unknown process option");
        lua_pop(L, 1);
    }
}

static void
raw_option(lua_State *L, int index, const char *name)
{
    index = lua_absindex(L, index);
    lua_pushstring(L, name);
    lua_rawget(L, index);
}

static int
parse_creation_relation(lua_State *L, int argument)
{
    size_t length;
    const char *relation = luaL_checklstring(L, argument, &length);
    if (strlen(relation) != length)
        return luaL_error(L, "unknown process relation");
    if (strcmp(relation, "creation") == 0) return 1;
    if (strcmp(relation, "ownership") == 0) return 0;
    return luaL_error(L, "unknown process relation");
}

static int active_in_tree(lua_Integer root, int attached_only);

static int
signal_process(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
    ProcessState state = parse_signal(L, 2);
    if (state == PROCESS_KILLED && luaL_testudata(L, 1, "malvado.process")) {
        MvProcessStatus status;
        if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK) {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    int self_kill = 0;
    if (state == PROCESS_KILLED && engine.active && L == engine.active_thread) {
        self_kill = active_in_tree(id, 1);
        if (self_kill < 0)
            return luaL_error(L, "cannot inspect process tree");
    }
    if (self_kill && !lua_isyieldable(L))
        return luaL_error(L, "cannot kill the active process in a non-yieldable context");
    if (mv_process_signal(engine.context, (MvProcessId)id, core_signal(state), 0) != MV_OK)
        return luaL_error(L, "unknown process id");
    sync_processes();
    if (self_kill)
        return lua_yield(L, 0);
    if (state == PROCESS_KILLED) {
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

/* Check the same live ownership tree that the current C signal API uses. */
static int
active_in_tree(lua_Integer root, int attached_only)
{
    size_t total = 0;
    if (mv_process_snapshot(engine.context, NULL, 0, &total) != MV_OK)
        return -1;
    if (total > SIZE_MAX / sizeof(MvProcessInfo))
        return -1;
    MvProcessInfo *items = total ? malloc(total * sizeof(*items)) : NULL;
    if (total && !items)
        return -1;
    size_t count = 0;
    if (mv_process_snapshot(engine.context, items, total, &count) != MV_OK || count > total) {
        free(items);
        return -1;
    }
    MvProcessId id = (MvProcessId)engine.active_id;
    int found = 0;
    for (size_t depth = 0; id && depth < count; ++depth) {
        if (id == (MvProcessId)root) {
            found = 1;
            break;
        }
        MvProcessId parent = 0;
        for (size_t i = 0; i < count; ++i) {
            if (items[i].id == id) {
                parent = (!attached_only || items[i].attached) ? items[i].parent : 0;
                break;
            }
        }
        id = parent;
    }
    free(items);
    return found;
}

static int
active_in_creation_tree(lua_Integer root)
{
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo)) return -1;
    MvProcessInfo *all = count ? malloc(count * sizeof(*all)) : NULL;
    MvProcessInfo *targets = count ? malloc(count * sizeof(*targets)) : NULL;
    if (count && (!all || !targets)) {
        free(all);
        free(targets);
        return -1;
    }
    size_t total = 0, target_count = 0;
    MvResult snapshot = mv_process_snapshot(engine.context, all, count, &total);
    MvResult result = mv_process_creation_descendants(engine.context, (MvProcessId)root, 0,
                                                       targets, count, &target_count);
    if (snapshot != MV_OK || result != MV_OK || total > count || target_count > count) {
        free(all);
        free(targets);
        return -1;
    }
    MvProcessId current = (MvProcessId)engine.active_id;
    int found = 0;
    for (size_t depth = 0; current && depth < total; ++depth) {
        if (current == (MvProcessId)root) found = 1;
        for (size_t i = 0; i < target_count && !found; ++i)
            if (targets[i].id == current) found = 1;
        if (found) break;
        MvProcessId owner = 0;
        for (size_t i = 0; i < total; ++i) {
            if (all[i].id == current) {
                if (all[i].attached) owner = all[i].parent;
                break;
            }
        }
        current = owner;
    }
    free(all);
    free(targets);
    return found;
}

static int
signal_tree(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
    ProcessState state = parse_signal(L, 2);
    int creation = 1;
    if (!lua_isnoneornil(L, 3)) {
        validate_options(L, 3, "relation", NULL);
        raw_option(L, 3, "relation");
        if (!lua_isnil(L, -1)) {
            creation = parse_creation_relation(L, -1);
        }
        lua_pop(L, 1);
    }
    int self_kill = 0;
    if (state == PROCESS_KILLED && engine.active && L == engine.active_thread) {
        self_kill = creation ? active_in_creation_tree(id) : active_in_tree(id, 0);
        if (self_kill < 0)
            return luaL_error(L, "cannot inspect process tree");
        if (self_kill && !lua_isyieldable(L))
            return luaL_error(L, "cannot kill the active process in a non-yieldable context");
    }
    MvResult result = creation ?
        mv_process_signal_creation_tree(engine.context, (MvProcessId)id, core_signal(state)) :
        mv_process_signal(engine.context, (MvProcessId)id, core_signal(state), 1);
    if (result == MV_OUT_OF_MEMORY) return luaL_error(L, "out of memory signaling process tree");
    if (result == MV_OVERFLOW) return luaL_error(L, "process tree is too large");
    if (result != MV_OK) return luaL_error(L, "unknown process id");
    sync_processes();
    if (self_kill) return lua_yield(L, 0);
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
    lua_Integer id = check_process_id(L, 1);
    lua_Integer priority = luaL_checkinteger(L, 2);
    if (mv_process_set_priority(engine.context, (MvProcessId)id, priority) != MV_OK)
        return luaL_error(L, "unknown process id");
    sync_processes();
    return 0;
}

static int
get_priority(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
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
process_alive(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
    MvProcessStatus status;
    lua_pushboolean(L, mv_process_status(engine.context, (MvProcessId)id, &status) == MV_OK);
    return 1;
}

static int
process_status(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK) {
        lua_pushnil(L);
        return 1;
    }
    const char *states[] = {"running", "sleeping", "frozen"};
    lua_pushstring(L, states[status]);
    return 1;
}

static void push_process_handle(lua_State *L, const MvProcessInfo *info);

static void
push_owner(lua_State *L, lua_Integer id)
{
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo))
        luaL_error(L, "too many processes");
    MvProcessInfo *items = lua_newuserdatauv(L, count * sizeof(*items), 0);
    size_t total;
    if (mv_process_snapshot(engine.context, items, count, &total) != MV_OK)
        luaL_error(L, "failed to inspect processes");
    MvProcessId owner = 0;
    for (size_t i = 0; i < count && i < total; ++i)
        if (items[i].id == (MvProcessId)id) owner = items[i].parent;
    for (size_t i = 0; i < count && i < total; ++i) {
        if (items[i].id == owner) {
            push_process_handle(L, &items[i]);
            lua_remove(L, -2);
            return;
        }
    }
    lua_pop(L, 1);
    lua_pushnil(L);
}

static int
self_index(lua_State *L)
{
    LuaSelf *self = luaL_checkudata(L, 1, "malvado.process");
    (void)check_process_id(L, 1);
    const char *key = lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : NULL;
    if (key && strcmp(key, "id") == 0) {
        lua_pushinteger(L, self->id);
        return 1;
    }
    MvProcessStatus current_status;
    if (mv_process_status(engine.context, (MvProcessId)self->id, &current_status) != MV_OK) {
        if (key && strcmp(key, "status") == 0) {
            lua_pushliteral(L, "dead");
            return 1;
        }
        if (key && strcmp(key, "reason") == 0) {
            MvEndReason ended;
            if (mv_process_end_reason(engine.context, (MvProcessId)self->id, &ended))
                self->reason = (int)ended;
            const char *reason = self->reason == 1 ? "returned" :
                                 self->reason == 2 ? "error" : "killed";
            lua_pushstring(L, reason);
            return 1;
        }
        return luaL_error(L, "process is no longer alive");
    }
    if (key && strcmp(key, "reason") == 0) {
        lua_pushnil(L);
        return 1;
    }
    if (key && strcmp(key, "creator_id") == 0) {
        if (self->parent) lua_pushinteger(L, self->parent);
        else lua_pushnil(L);
        return 1;
    }
    if (key && strcmp(key, "parent") == 0) {
        push_owner(L, self->id);
        return 1;
    }
    if (key && strcmp(key, "priority") == 0) {
        int64_t priority;
        if (mv_process_priority(engine.context, (MvProcessId)self->id, &priority) != MV_OK)
            return luaL_error(L, "process is no longer alive");
        lua_pushinteger(L, priority);
        return 1;
    }
    if (key && strcmp(key, "delta") == 0) {
        Process *process = find_process(self->id);
        if (!process)
            return luaL_error(L, "process is no longer alive");
        lua_pushnumber(L, process->delta);
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
    if (key && lua_isnil(L, -1)) {
        int zero = !strcmp(key, "x") || !strcmp(key, "y") || !strcmp(key, "z") ||
                   !strcmp(key, "angle") || !strcmp(key, "fps") ||
                   !strcmp(key, "width") || !strcmp(key, "height");
        if (zero || !strcmp(key, "size")) {
            lua_pop(L, 1);
            lua_pushinteger(L, zero ? 0 : 1);
        }
    }
    return 1;
}

static const char *const read_only_process_fields[] = {
    "id", "parent", "creator_id", "priority", "status", "reason", "recv", "delta", NULL
};

static int
is_read_only_process_field(const char *key)
{
    for (size_t i = 0; read_only_process_fields[i]; ++i)
        if (strcmp(key, read_only_process_fields[i]) == 0) return 1;
    return 0;
}

static int
self_newindex(lua_State *L)
{
    LuaSelf *self = luaL_checkudata(L, 1, "malvado.process");
    (void)check_process_id(L, 1);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)self->id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    const char *key = lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : NULL;
    if (key && is_read_only_process_field(key))
        return luaL_error(L, "process field is read-only");
    if (key && strcmp(key, "z") == 0 &&
        (lua_type(L, 3) != LUA_TNUMBER || !isfinite(lua_tonumber(L, 3))))
        return luaL_error(L, "z must be a finite number");
    if (key && strcmp(key, "children_attached") == 0 && lua_type(L, 3) != LUA_TBOOLEAN)
        return luaL_error(L, "children_attached must be a boolean");
    if (key && strcmp(key, "fps") == 0 &&
        (!lua_isinteger(L, 3) || lua_tointeger(L, 3) < 0 || lua_tointeger(L, 3) > 60))
        return luaL_error(L, "fps must be an integer in 0..60");
    lua_getiuservalue(L, 1, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    return 0;
}

/* Copy table graphs without sharing mutable defaults between processes. */
static void
copy_default(lua_State *L, int source, int copies, unsigned depth)
{
    source = lua_absindex(L, source);
    copies = lua_absindex(L, copies);
    if (depth > 256 || !lua_checkstack(L, 8))
        luaL_error(L, "defaults are too deeply nested");
    int type = lua_type(L, source);
    if (type == LUA_TUSERDATA || type == LUA_TLIGHTUSERDATA || type == LUA_TTHREAD)
        luaL_error(L, "defaults cannot contain userdata or threads");
    if (type != LUA_TTABLE) {
        lua_pushvalue(L, source);
        return;
    }
    if (lua_getmetatable(L, source))
        luaL_error(L, "defaults cannot contain metatables");
    lua_pushvalue(L, source);
    lua_rawget(L, copies);
    if (!lua_isnil(L, -1))
        return;
    lua_pop(L, 1);
    lua_newtable(L);
    int result = lua_gettop(L);
    lua_pushvalue(L, source);
    lua_pushvalue(L, result);
    lua_rawset(L, copies);
    lua_pushnil(L);
    while (lua_next(L, source)) {
        copy_default(L, -2, copies, depth + 1);
        copy_default(L, -2, copies, depth + 1);
        lua_rawset(L, result);
        lua_pop(L, 1);
    }
}

static int
create_process(lua_State *L)
{
    lua_settop(L, 2);
    if (!lua_isnil(L, 1)) luaL_checktype(L, 1, LUA_TTABLE);
    if (!lua_isnil(L, 2)) validate_options(L, 2, "attached", NULL);
    int explicit_attachment = 0, attached = 1;
    if (lua_istable(L, 2)) {
        raw_option(L, 2, "attached");
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TBOOLEAN);
            explicit_attachment = 1;
            attached = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
    }
    lua_newtable(L);
    int copies = lua_gettop(L);
    if (lua_isnil(L, lua_upvalueindex(3)))
        lua_newtable(L);
    else
        copy_default(L, lua_upvalueindex(3), copies, 0);
    int fields = lua_gettop(L);
    if (lua_istable(L, 1)) {
        lua_pushnil(L);
        while (lua_next(L, 1)) {
            lua_pushvalue(L, -2);
            lua_pushvalue(L, -2);
            lua_rawset(L, fields);
            lua_pop(L, 1);
        }
    }
    for (size_t i = 0; read_only_process_fields[i]; ++i) {
        lua_pushstring(L, read_only_process_fields[i]);
        lua_rawget(L, fields);
        if (!lua_isnil(L, -1))
            return luaL_error(L, "process field is read-only: %s", read_only_process_fields[i]);
        lua_pop(L, 1);
    }
    lua_getfield(L, fields, "children_attached");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushvalue(L, lua_upvalueindex(4));
        lua_setfield(L, fields, "children_attached");
    } else {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        lua_pop(L, 1);
    }
    lua_pushliteral(L, "z");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) &&
        (lua_type(L, -1) != LUA_TNUMBER || !isfinite(lua_tonumber(L, -1))))
        return luaL_error(L, "z must be a finite number");
    lua_pop(L, 1);
    lua_pushliteral(L, "fps");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) &&
        (!lua_isinteger(L, -1) || lua_tointeger(L, -1) < 0 || lua_tointeger(L, -1) > 60))
        return luaL_error(L, "fps must be an integer in 0..60");
    lua_pop(L, 1);
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
    lua_Integer creator = engine.closing_id ? engine.closing_id : engine.active_id;
    if (!creator && explicit_attachment && attached) {
        free(handle);
        return luaL_error(L, "cannot attach a process without an owner");
    }
    if (engine.closing_id && (!explicit_attachment || attached)) {
        free(handle);
        return luaL_error(L, "a terminating process can only spawn independent processes explicitly");
    }
    if (creator && !explicit_attachment && !engine.closing_id) {
        Process *owner = find_process(creator);
        if (!owner) {
            free(handle);
            return luaL_error(L, "process creator is no longer alive");
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, owner->self_ref);
        lua_getiuservalue(L, -1, 1);
        lua_getfield(L, -1, "children_attached");
        attached = lua_toboolean(L, -1);
        lua_pop(L, 3);
    }
    MvProcessId parent = attached ? (MvProcessId)creator : 0;
    MvResult spawn_result = mv_process_spawn_related(engine.context, (MvProcessId)creator, parent,
                                                     0, lua_process_step, lua_process_destroy,
                                                     handle, &core_id);
    if (spawn_result != MV_OK) {
        free(handle);
        return luaL_error(L, "failed to create process");
    }
    handle->id = (lua_Integer)core_id;
    (void)mv_process_set_type(engine.context, core_id, (uint64_t)lua_tointeger(L, lua_upvalueindex(2)));
    engine.next_process_id = handle->id;
    lua_Integer id = handle->id;
    LuaSelf *self = lua_newuserdatauv(L, sizeof(*self), 1);
    *self = (LuaSelf){.id = id, .parent = creator};
    if (mv_process_ref_create(engine.context, core_id, &self->reference) != MV_OK)
        return luaL_error(L, "failed to create process identity");
    lua_pushvalue(L, fields);
    lua_setiuservalue(L, -2, 1);
    luaL_getmetatable(L, "malvado.process");
    lua_setmetatable(L, -2);
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.process_handles");
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, id);
    lua_pop(L, 1);
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
        .parent_id = (lua_Integer)parent,
        .state = PROCESS_RUNNING,
        .first = 1,
        .alive = 1
    };
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    return 1;
}

static int
define_process(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (engine.next_type_id == LUA_MAXINTEGER)
        return luaL_error(L, "process type ID space exhausted");
    if (!lua_isnoneornil(L, 2)) {
        validate_options(L, 2, "defaults", "children_attached");
        raw_option(L, 2, "defaults");
        if (!lua_isnil(L, -1)) luaL_checktype(L, -1, LUA_TTABLE);
        lua_pop(L, 1);
        raw_option(L, 2, "children_attached");
        if (!lua_isnil(L, -1)) luaL_checktype(L, -1, LUA_TBOOLEAN);
        lua_pop(L, 1);
    }
    lua_settop(L, 2);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, ++engine.next_type_id);
    if (lua_istable(L, 2)) {
        raw_option(L, 2, "defaults");
        if (!lua_isnil(L, -1))
            luaL_checktype(L, -1, LUA_TTABLE);
    } else {
        lua_pushnil(L);
    }
    if (lua_istable(L, 2)) {
        raw_option(L, 2, "children_attached");
        if (!lua_isnil(L, -1)) luaL_checktype(L, -1, LUA_TBOOLEAN);
        else { lua_pop(L, 1); lua_pushboolean(L, 1); }
    } else lua_pushboolean(L, 1);
    lua_pushcclosure(L, create_process, 4);
    return 1;
}

/* Results are copies, never references to mutable scheduler storage. */
static const char *
state_name(ProcessState state)
{
    static const char *names[] = {"running", "sleeping", "frozen", "killed"};
    return names[state];
}

static void
push_process_handle(lua_State *L, const MvProcessInfo *info)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.process_handles");
    lua_rawgeti(L, -1, (lua_Integer)info->id);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);
    LuaSelf *self = lua_newuserdatauv(L, sizeof(*self), 1);
    *self = (LuaSelf){.id = (lua_Integer)info->id, .parent = (lua_Integer)info->creator_id};
    if (mv_process_ref_create(engine.context, info->id, &self->reference) != MV_OK)
        luaL_error(L, "failed to create process identity");
    lua_newtable(L);
    lua_setiuservalue(L, -2, 1);
    luaL_getmetatable(L, "malvado.process");
    lua_setmetatable(L, -2);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, (lua_Integer)info->id);
    lua_remove(L, -2);
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
            push_process_handle(L, &items[i]);
            lua_seti(L, -2, ++result);
        }
    }
    return 1;
}

static int
hierarchy_query(lua_State *L, int direct)
{
    lua_Integer root = check_process_id(L, 1);
    int creation = 0;
    uint64_t type = 0;
    if (!lua_isnoneornil(L, 2)) {
        validate_options(L, 2, "relation", "type");
        raw_option(L, 2, "relation");
        if (!lua_isnil(L, -1)) {
            creation = parse_creation_relation(L, -1);
        }
        lua_pop(L, 1);
        raw_option(L, 2, "type");
        if (!lua_isnil(L, -1)) {
            if (!lua_iscfunction(L, -1) || lua_tocfunction(L, -1) != create_process ||
                !lua_getupvalue(L, -1, 2))
                return luaL_error(L, "expected a process constructor for type");
            type = (uint64_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    size_t count = mv_process_count(engine.context);
    if (count > SIZE_MAX / sizeof(MvProcessInfo))
        return luaL_error(L, "too many processes");
    MvProcessInfo *items = lua_newuserdatauv(L, count * sizeof(*items), 0);
    size_t total = 0;
    MvResult result;
    if (creation)
        result = mv_process_creation_descendants(engine.context, (MvProcessId)root, type,
                                                 items, count, &total);
    else if (direct)
        result = mv_process_children(engine.context, (MvProcessId)root, type, items, count, &total);
    else
        result = mv_process_descendants(engine.context, (MvProcessId)root, type, items, count, &total);
    if (result != MV_OK) return luaL_error(L, "invalid process query root");
    lua_newtable(L);
    lua_Integer output = 0;
    for (size_t i = 0; i < total && i < count; ++i) {
        if (creation && direct && items[i].creator_id != (MvProcessId)root) continue;
        push_process_handle(L, &items[i]);
        lua_seti(L, -2, ++output);
    }
    return 1;
}

static int
process_children(lua_State *L)
{
    return hierarchy_query(L, 1);
}

static int
process_descendants(lua_State *L)
{
    return hierarchy_query(L, 0);
}

static int
snapshot(lua_State *L)
{
    sync_processes();
    size_t count = 0, lineage_count = 0;
    MvResult probe = mv_process_inspection_snapshot(engine.context, NULL, 0, &count,
                                                    NULL, 0, &lineage_count);
    if (probe != MV_OK && probe != MV_BUFFER_TOO_SMALL)
        return luaL_error(L, "failed to inspect processes");
    MvProcessInfo *items = NULL;
    MvLineageInfo *lineage = NULL;
    size_t *draw_counts = NULL;
    size_t total = 0, lineage_total = 0;
    int captured = 0;
    for (int attempt = 0; attempt < 16 && !captured; ++attempt) {
        if (count > SIZE_MAX / sizeof(*items) || count > SIZE_MAX / sizeof(*draw_counts) ||
            lineage_count > SIZE_MAX / sizeof(*lineage) || !lua_checkstack(L, 8))
            return luaL_error(L, "inspection snapshot is too large");
        items = lua_newuserdatauv(L, count * sizeof(*items), 0);
        lineage = lua_newuserdatauv(L, lineage_count * sizeof(*lineage), 0);
        draw_counts = lua_newuserdatauv(L, count * sizeof(*draw_counts), 0);
        MvResult result = mv_process_inspection_snapshot(engine.context, items, count, &total,
                                                         lineage, lineage_count, &lineage_total);
        if (result == MV_BUFFER_TOO_SMALL) {
            lua_pop(L, 3);
            count = total;
            lineage_count = lineage_total;
        } else if (result == MV_OK) {
            captured = 1;
        } else {
            return luaL_error(L, "failed to inspect processes");
        }
    }
    if (!captured) return luaL_error(L, "process list changed during inspection");
    for (size_t i = 0; i < total; ++i) {
        Process *process = find_process_slot((lua_Integer)items[i].id);
        int draw_ref = process ? process->draw_ref : LUA_NOREF;
        lua_rawgeti(L, LUA_REGISTRYINDEX, draw_ref);
        draw_counts[i] = lua_rawlen(L, -1);
        lua_pop(L, 1);
    }
    MvRuntimeId runtime_id;
    if (mv_context_runtime_id(engine.context, &runtime_id) != MV_OK)
        return luaL_error(L, "failed to inspect runtime identity");
    char runtime_text[49];
    (void)snprintf(runtime_text, sizeof(runtime_text), "%016" PRIx64 "%016" PRIx64 "%016" PRIx64,
                   runtime_id.high, runtime_id.low, runtime_id.sequence);
    lua_newtable(L);
    lua_pushstring(L, runtime_text);
    lua_setfield(L, -2, "runtime");
    lua_pushinteger(L, engine.tick_number);
    lua_setfield(L, -2, "tick");
    lua_newtable(L);
    lua_Integer result = 0;
    for (size_t i = 0; i < count && i < total; i++) {
        MvProcessInfo *p = &items[i];
        lua_createtable(L, 0, 8);
        lua_pushinteger(L, (lua_Integer)p->id); lua_setfield(L, -2, "id");
        if (p->parent) {
            lua_pushinteger(L, (lua_Integer)p->parent); lua_setfield(L, -2, "parent");
        }
        if (p->creator_id) {
            lua_pushinteger(L, (lua_Integer)p->creator_id); lua_setfield(L, -2, "creator_id");
        }
        lua_pushboolean(L, p->attached); lua_setfield(L, -2, "attached");
        lua_pushinteger(L, (lua_Integer)p->type); lua_setfield(L, -2, "type");
        lua_pushinteger(L, p->priority); lua_setfield(L, -2, "priority");
        lua_pushstring(L, state_name((ProcessState)p->status)); lua_setfield(L, -2, "status");
        lua_pushinteger(L, (lua_Integer)p->messages); lua_setfield(L, -2, "messages");
        lua_pushinteger(L, (lua_Integer)draw_counts[i]); lua_setfield(L, -2, "draw_commands");
        lua_pushboolean(L, p->status != MV_PROCESS_SLEEPING); lua_setfield(L, -2, "visible");
        lua_seti(L, -2, ++result);
    }
    lua_setfield(L, -2, "processes");
    lua_newtable(L);
    for (size_t i = 0; i < lineage_total && i < lineage_count; ++i) {
        lua_createtable(L, 0, 3);
        lua_pushinteger(L, (lua_Integer)lineage[i].id);
        lua_setfield(L, -2, "id");
        if (lineage[i].creator_id) {
            lua_pushinteger(L, (lua_Integer)lineage[i].creator_id);
            lua_setfield(L, -2, "creator_id");
        }
        lua_pushboolean(L, lineage[i].alive);
        lua_setfield(L, -2, "alive");
        lua_seti(L, -2, (lua_Integer)i + 1);
    }
    lua_setfield(L, -2, "lineage");
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
queue_fade(lua_State *L, MvColor color, unsigned ticks)
{
    if (!mv_fade_active(&engine.fade) && engine.fade_count == 0) {
        (void)mv_fade_start(&engine.fade, color, ticks);
        return 0;
    }
    if (engine.fade_count == 16)
        return luaL_error(L, "fade queue is full");
    unsigned index = (engine.fade_head + engine.fade_count) % 16;
    engine.fade_queue[index] = (FadeRequest){color, ticks};
    ++engine.fade_count;
    return 0;
}

static void
advance_fade(void)
{
    (void)mv_fade_step(&engine.fade);
    while (!mv_fade_active(&engine.fade) && engine.fade_count > 0) {
        FadeRequest request = engine.fade_queue[engine.fade_head];
        engine.fade_head = (engine.fade_head + 1) % 16;
        --engine.fade_count;
        (void)mv_fade_start(&engine.fade, request.target, request.ticks);
    }
}

static int
fade_to(lua_State *L)
{
    lua_Integer values[5];
    for (int i = 0; i < 5; ++i) {
        values[i] = luaL_checkinteger(L, i + 1);
        lua_Integer limit = i == 4 ? 1000000 : 255;
        if (values[i] < 0 || values[i] > limit)
            return luaL_error(L, "invalid fade color or duration");
    }
    return queue_fade(L, (MvColor){values[0], values[1], values[2], values[3]},
                      (unsigned)values[4]);
}

static int
fade_switch(lua_State *L)
{
    lua_Integer ticks = luaL_optinteger(L, 1, 16);
    if (ticks < 0 || ticks > 1000000)
        return luaL_error(L, "invalid fade duration");
    int opaque = lua_toboolean(L, lua_upvalueindex(1));
    return queue_fade(L, (MvColor){0, 0, 0, opaque ? 255 : 0}, (unsigned)ticks);
}

static int
fade_active(lua_State *L)
{
    lua_pushboolean(L, mv_fade_active(&engine.fade) || engine.fade_count != 0);
    return 1;
}

static int
fade_color(lua_State *L)
{
    MvColor color = engine.fade.current;
    lua_pushinteger(L, color.r);
    lua_pushinteger(L, color.g);
    lua_pushinteger(L, color.b);
    lua_pushinteger(L, color.a);
    return 4;
}

static int
set_color(lua_State *L)
{
    lua_Integer values[4];
    for (int i = 0; i < 4; ++i) {
        values[i] = i == 3 ? luaL_optinteger(L, i + 1, 255) : luaL_checkinteger(L, i + 1);
        if (values[i] < 0 || values[i] > 255)
            return luaL_error(L, "color must be in 0..255");
    }
    engine.draw_color = (MvColor){values[0], values[1], values[2], values[3]};
    return 0;
}

static int
draw_primitive(lua_State *L)
{
    int type = (int)lua_tointeger(L, lua_upvalueindex(1));
    int count = type <= 2 ? 4 : 3;
    int64_t values[4] = {0};
    for (int i = 0; i < count; ++i) {
        lua_Integer value = luaL_checkinteger(L, i + 1);
        if (value < INT_MIN || value > INT_MAX)
            return luaL_error(L, "primitive coordinate is outside the supported range");
        values[i] = (int64_t)value;
    }
    if (type <= 2) {
        values[2] -= values[0];
        values[3] -= values[1];
        if (values[2] <= 0 || values[2] > INT_MAX || values[3] <= 0 || values[3] > INT_MAX)
            return luaL_error(L, "invalid rectangle");
    } else if (values[2] <= 0 || values[0] + values[2] > INT_MAX ||
               values[0] - values[2] < INT_MIN || values[1] + values[2] > INT_MAX ||
               values[1] - values[2] < INT_MIN) {
        return luaL_error(L, "invalid circle");
    }
    if (!engine.active || L != engine.active_thread)
        return luaL_error(L, "drawing must be called inside a process");
    Process *process = find_process(engine.active_id);
    if (!process)
        return luaL_error(L, "drawing must be called inside a live process");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
    lua_createtable(L, 9, 0);
    lua_pushinteger(L, type);
    lua_seti(L, -2, 1);
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, (lua_Integer)values[i]);
        lua_seti(L, -2, i + 2);
    }
    const unsigned char channels[] = {
        engine.draw_color.r, engine.draw_color.g, engine.draw_color.b, engine.draw_color.a
    };
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, channels[i]);
        lua_seti(L, -2, i + 6);
    }
    lua_seti(L, -2, lua_rawlen(L, -2) + 1);
    lua_pop(L, 1);
    return 0;
}

static int
window_size(lua_State *L)
{
    if (!engine.started || engine.width <= 0 || engine.height <= 0)
        return luaL_error(L, "window size is not available before start");
    int width = engine.width, height = engine.height;
    if (engine.window && !mv_window_size(&width, &height))
        return luaL_error(L, "window is not available");
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int
legacy_screen_dimension(lua_State *L)
{
    int dimension = (int)lua_tointeger(L, lua_upvalueindex(1));
    window_size(L);
    lua_pushvalue(L, dimension == 0 ? -2 : -1);
    return 1;
}

static int
set_title(lua_State *L)
{
    size_t length;
    const char *title = luaL_checklstring(L, 1, &length);
    if (strlen(title) != length)
        return luaL_error(L, "title contains a zero byte");
    if (!engine.window || !mv_window_set_title(title))
        return luaL_error(L, "window is not available");
    return 0;
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
pad_slot(lua_State *L)
{
    lua_Integer slot = luaL_checkinteger(L, 1);
    if (slot < 0 || slot > 3)
        luaL_error(L, "controller slot must be in 0..3");
    return (int)slot;
}

static int
pad_connected(lua_State *L)
{
    int slot = pad_slot(L);
    lua_pushboolean(L, !engine.headless && mv_pad_connected(slot));
    return 1;
}

static int
pad_button(lua_State *L)
{
    int slot = pad_slot(L);
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    MvPadButton button;
    if (strlen(name) != length || !mv_pad_button_from_name(name, &button))
        return luaL_error(L, "unsupported controller button");
    lua_pushboolean(L, !engine.headless && mv_pad_button_down(slot, button));
    return 1;
}

static int
pad_axis(lua_State *L)
{
    int slot = pad_slot(L);
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    MvPadAxis axis;
    if (strlen(name) != length || !mv_pad_axis_from_name(name, &axis))
        return luaL_error(L, "unsupported controller axis");
    lua_pushnumber(L, engine.headless ? 0 : mv_pad_axis(slot, axis));
    return 1;
}

static int
text_input_next(lua_State *L)
{
    uint32_t codepoint;
    if (engine.headless || !mv_text_input_next(&codepoint)) {
        lua_pushnil(L);
        return 1;
    }
    char bytes[4];
    size_t length = mv_utf8_encode(codepoint, bytes);
    if (!length)
        return luaL_error(L, "invalid text input character");
    lua_pushlstring(L, bytes, length);
    return 1;
}

static MvResource *
manage_native_resource(lua_State *L, void *data, MvResourceDestroy destroy)
{
    MvResource *resource = NULL;
    MvProcessRef owner;
    const MvProcessRef *owner_ref = NULL;
    if (engine.active) {
        if (mv_process_ref_create(engine.context, (MvProcessId)engine.active_id, &owner) != MV_OK) {
            destroy(data);
            luaL_error(L, "could not identify resource owner");
        }
        owner_ref = &owner;
    }
    if (mv_resource_create(engine.context, owner_ref, data, destroy, &resource) != MV_OK) {
        destroy(data);
        luaL_error(L, "could not manage native resource");
    }
    if (mv_resource_retain(resource) != MV_OK)
        luaL_error(L, "could not retain native resource");
    return resource;
}

static void destroy_managed_sound(void *data) { mv_sound_destroy(data); }
static void destroy_managed_music(void *data) { mv_music_destroy(data); }
static void destroy_managed_font(void *data) { mv_font_destroy(data); }

/* Keep a playing or paused stream alive until it stops or the runtime closes. */
static void
pin_audio(lua_State *L, int pinned)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.playing_audio");
    lua_pushvalue(L, 1);
    if (pinned) lua_pushboolean(L, 1);
    else lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

static void
sweep_audio(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.playing_audio");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        LuaSound *sound = luaL_testudata(L, -2, "malvado.sound");
        LuaMusic *music = luaL_testudata(L, -2, "malvado.music");
        int ended = sound ? sound->playing && !sound->paused && !mv_sound_playing(sound->sound) :
                    music ? music->playing && !music->paused && !mv_music_playing(music->music) : 1;
        if (ended) {
            if (sound) sound->playing = 0;
            if (music) music->playing = 0;
            lua_pushvalue(L, -2);
            lua_pushnil(L);
            lua_rawset(L, -5);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static int
sound_gc(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    if (sound->resource) mv_resource_release(sound->resource);
    else mv_sound_destroy(sound->sound);
    sound->resource = NULL;
    sound->sound = NULL;
    return 0;
}

static int
load_sound(lua_State *L)
{
    size_t length;
    const char *path = luaL_checklstring(L, 1, &length);
    if (strlen(path) != length)
        return luaL_error(L, "sound path contains a zero byte");
    if (engine.headless)
        return luaL_error(L, "sound is not available in headless mode");
    if (!engine.audio) {
        if (!mv_audio_open())
            return luaL_error(L, "could not open audio device");
        engine.audio = 1;
    }
    LuaSound *sound = lua_newuserdatauv(L, sizeof(*sound), 0);
    *sound = (LuaSound){0};
    luaL_setmetatable(L, "malvado.sound");
    sound->sound = mv_sound_load(path);
    if (!sound->sound)
        return luaL_error(L, "could not load WAV sound: %s", path);
    MvSound *native = sound->sound;
    sound->sound = NULL;
    sound->resource = manage_native_resource(L, native, destroy_managed_sound);
    sound->sound = native;
    return 1;
}

static int
clone_sound(lua_State *L)
{
    LuaSound *source = luaL_checkudata(L, 1, "malvado.sound");
    LuaSound *copy = lua_newuserdatauv(L, sizeof(*copy), 0);
    *copy = (LuaSound){0};
    luaL_setmetatable(L, "malvado.sound");
    copy->sound = mv_sound_clone(source->sound);
    if (!copy->sound)
        return luaL_error(L, "could not create sound instance");
    MvSound *native = copy->sound;
    copy->sound = NULL;
    copy->resource = manage_native_resource(L, native, destroy_managed_sound);
    copy->sound = native;
    return 1;
}

static int
play_sound(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    if (!mv_sound_play(sound->sound))
        return luaL_error(L, "sound is not available");
    sound->playing = 1;
    sound->paused = 0;
    pin_audio(L, 1);
    return 0;
}

static int
stop_sound(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    if (!mv_sound_stop(sound->sound))
        return luaL_error(L, "sound is not available");
    sound->playing = sound->paused = 0;
    pin_audio(L, 0);
    return 0;
}

static int
pause_sound(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    if (!mv_sound_pause(sound->sound))
        return luaL_error(L, "sound is not available");
    if (sound->playing) sound->paused = 1;
    return 0;
}

static int
resume_sound(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    if (!mv_sound_resume(sound->sound))
        return luaL_error(L, "sound is not available");
    if (sound->playing) sound->paused = 0;
    return 0;
}

static int
sound_playing(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    lua_pushboolean(L, mv_sound_playing(sound->sound));
    return 1;
}

static int
sound_volume(lua_State *L)
{
    LuaSound *sound = luaL_checkudata(L, 1, "malvado.sound");
    lua_Number volume = luaL_checknumber(L, 2);
    if (!isfinite(volume) || volume < 0 || volume > 1)
        return luaL_error(L, "volume must be in 0..1");
    if (!mv_sound_set_volume(sound->sound, (float)volume))
        return luaL_error(L, "sound is not available");
    return 0;
}

static int
music_gc(lua_State *L)
{
    LuaMusic *music = luaL_checkudata(L, 1, "malvado.music");
    if (music->resource) mv_resource_release(music->resource);
    else mv_music_destroy(music->music);
    music->resource = NULL;
    music->music = NULL;
    return 0;
}

static int
load_music(lua_State *L)
{
    size_t length;
    const char *path = luaL_checklstring(L, 1, &length);
    if (strlen(path) != length)
        return luaL_error(L, "music path contains a zero byte");
    if (!lua_isnoneornil(L, 2) && !lua_isboolean(L, 2))
        return luaL_error(L, "loop must be a boolean");
    int loop = lua_isnoneornil(L, 2) ? 0 : lua_toboolean(L, 2);
    if (engine.headless)
        return luaL_error(L, "music is not available in headless mode");
    if (!engine.audio) {
        if (!mv_audio_open())
            return luaL_error(L, "could not open audio device");
        engine.audio = 1;
    }
    LuaMusic *music = lua_newuserdatauv(L, sizeof(*music), 0);
    *music = (LuaMusic){0};
    luaL_setmetatable(L, "malvado.music");
    music->music = mv_music_load(path, loop);
    if (!music->music)
        return luaL_error(L, "could not load music: %s", path);
    MvMusic *native = music->music;
    music->music = NULL;
    music->resource = manage_native_resource(L, native, destroy_managed_music);
    music->music = native;
    return 1;
}

static int
music_action(lua_State *L)
{
    LuaMusic *music = luaL_checkudata(L, 1, "malvado.music");
    int action = (int)lua_tointeger(L, lua_upvalueindex(1));
    int ok = 0;
    switch (action) {
    case 0: ok = mv_music_play(music->music); break;
    case 1: ok = mv_music_update(music->music); break;
    case 2: ok = mv_music_stop(music->music); break;
    case 3: ok = mv_music_pause(music->music); break;
    case 4: ok = mv_music_resume(music->music); break;
    }
    if (!ok)
        return luaL_error(L, "music is not available");
    if (action == 0) {
        music->playing = 1;
        music->paused = 0;
        pin_audio(L, 1);
    } else if (action == 2) {
        music->playing = music->paused = 0;
        pin_audio(L, 0);
    } else if (action == 3 && music->playing) music->paused = 1;
    else if (action == 4 && music->playing) music->paused = 0;
    return 0;
}

static int
music_playing(lua_State *L)
{
    LuaMusic *music = luaL_checkudata(L, 1, "malvado.music");
    lua_pushboolean(L, mv_music_playing(music->music));
    return 1;
}

static int
music_volume(lua_State *L)
{
    LuaMusic *music = luaL_checkudata(L, 1, "malvado.music");
    lua_Number volume = luaL_checknumber(L, 2);
    if (!isfinite(volume) || volume < 0 || volume > 1)
        return luaL_error(L, "volume must be in 0..1");
    if (!mv_music_set_volume(music->music, (float)volume))
        return luaL_error(L, "music is not available");
    return 0;
}

static void
destroy_managed_image(void *data)
{
    mv_image_destroy(data);
}

static void
manage_image(lua_State *L, LuaImage *image)
{
    MvImage *native = image->image;
    image->image = NULL;
    image->resource = manage_native_resource(L, native, destroy_managed_image);
    image->image = native;
}

static int
image_gc(lua_State *L)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    if (image->resource) mv_resource_release(image->resource);
    else mv_image_destroy(image->image);
    image->resource = NULL;
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
    *image = (LuaImage){0};
    luaL_setmetatable(L, "malvado.image");
    image->image = mv_image_load(path);
    if (!image->image)
        return luaL_error(L, "could not load PNG image: %s", path);
    manage_image(L, image);
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

#ifndef _WIN32
static int
directory_entries_gc(lua_State *L)
{
    LuaDirectoryEntries *entries = luaL_checkudata(L, 1, "malvado.directory_entries");
    for (size_t i = 0; i < entries->count; ++i)
        free(entries->names[i]);
    free(entries->names);
    entries->names = NULL;
    entries->count = 0;
    return 0;
}

static int
compare_names(const void *left, const void *right)
{
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}

static int
png_name(const char *name)
{
    size_t length = strlen(name);
    return length > 4 && name[length - 4] == '.' &&
           (name[length - 3] == 'p' || name[length - 3] == 'P') &&
           (name[length - 2] == 'n' || name[length - 2] == 'N') &&
           (name[length - 1] == 'g' || name[length - 1] == 'G');
}

static int
legacy_directory(lua_State *L, const char *path)
{
    LuaDirectoryEntries *entries = lua_newuserdatauv(L, sizeof(*entries), 0);
    *entries = (LuaDirectoryEntries){0};
    luaL_setmetatable(L, "malvado.directory_entries");
    entries->names = calloc(4096, sizeof(*entries->names));
    if (!entries->names)
        return luaL_error(L, "out of memory reading graphics directory");
    DIR *directory = opendir(path);
    if (!directory)
        return luaL_error(L, "could not open graphics directory: %s", path);
    int failure = 0;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (!entry) {
            failure = errno != 0;
            break;
        }
        if (!png_name(entry->d_name))
            continue;
        if (entries->count == 4096) {
            failure = 1;
            break;
        }
        size_t length = strlen(entry->d_name);
        char *name = malloc(length + 1);
        if (!name) {
            failure = 1;
            break;
        }
        memcpy(name, entry->d_name, length + 1);
        entries->names[entries->count++] = name;
    }
    closedir(directory);
    if (failure || entries->count == 0)
        return luaL_error(L, "could not read PNG files from graphics directory: %s", path);
    qsort(entries->names, entries->count, sizeof(*entries->names), compare_names);
    lua_createtable(L, (int)entries->count, 0);
    int images = lua_absindex(L, -1);
    for (size_t i = 0; i < entries->count; ++i) {
        const char *file = lua_pushfstring(L, "%s/%s", path, entries->names[i]);
        LuaImage *image = lua_newuserdatauv(L, sizeof(*image), 0);
        *image = (LuaImage){0};
        luaL_setmetatable(L, "malvado.image");
        image->image = mv_image_load(file);
        if (!image->image)
            return luaL_error(L, "could not load PNG image: %s", file);
        manage_image(L, image);
        lua_seti(L, images, (lua_Integer)i + 1);
        lua_pop(L, 1);
    }
    LuaLegacySheet *sheet = lua_newuserdatauv(L, sizeof(*sheet), 1);
    *sheet = (LuaLegacySheet){0, 0, (int)entries->count, 1};
    luaL_setmetatable(L, "malvado.legacy_sheet");
    lua_pushvalue(L, images);
    lua_setiuservalue(L, -2, 1);
    return 1;
}
#endif

static int
legacy_fpg(lua_State *L)
{
    size_t path_length;
    const char *path = luaL_checklstring(L, 1, &path_length);
    if (path_length == 0 || strlen(path) != path_length)
        return luaL_error(L, "invalid graphics library path");
    size_t kind_length;
    const char *kind = luaL_optlstring(L, 2, "directory", &kind_length);
    if (kind_length == 9 && memcmp(kind, "directory", 9) == 0) {
        if (!engine.window)
            return luaL_error(L, "fpg requires an open window");
#ifdef _WIN32
        return luaL_error(L, "legacy directory graphics are not supported on Windows");
#else
        return legacy_directory(L, path);
#endif
    }
    if (kind_length != 5 || memcmp(kind, "image", 5) != 0)
        return luaL_error(L, "legacy fpg supports only image sheets");
    lua_Integer rows = luaL_checkinteger(L, 3);
    lua_Integer columns = luaL_checkinteger(L, 4);
    if (rows < 1 || rows > INT_MAX || columns < 1 || columns > INT_MAX)
        return luaL_error(L, "invalid sheet dimensions");
    load_image(L);
    LuaImage *image = luaL_checkudata(L, -1, "malvado.image");
    int width = mv_image_width(image->image);
    int height = mv_image_height(image->image);
    if (columns > width || rows > height || width % columns != 0 || height % rows != 0)
        return luaL_error(L, "sheet dimensions must divide the image");
    LuaLegacySheet *sheet = lua_newuserdatauv(L, sizeof(*sheet), 1);
    *sheet = (LuaLegacySheet){(int)rows, (int)columns, 0, 0};
    luaL_setmetatable(L, "malvado.legacy_sheet");
    lua_pushvalue(L, -2);
    lua_setiuservalue(L, -2, 1);
    return 1;
}

static lua_Number
legacy_number(lua_State *L, int table, const char *name, lua_Number fallback)
{
    lua_getfield(L, table, name);
    lua_Number value = lua_isnil(L, -1) ? fallback : luaL_checknumber(L, -1);
    lua_pop(L, 1);
    if (!isfinite(value))
        luaL_error(L, "legacy graphic fields must be finite");
    return value;
}

static int
legacy_frame(lua_State *L)
{
    if (!engine.active || L != engine.active_thread || !lua_isyieldable(L))
        return luaL_error(L, "frame() must be called inside a process");
    Process *process = find_process(engine.active_id);
    if (!process)
        return luaL_error(L, "frame() must be called inside a live process");
    int self_ref = process->self_ref;
    lua_settop(L, 0);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    lua_getiuservalue(L, 1, 1);
    lua_getfield(L, 2, "fpg");
    int x_source = 0, y_source = 0, region_width, region_height;
    int sheet_image = 0;
    if (!lua_isnil(L, 3)) {
        LuaLegacySheet *sheet = luaL_checkudata(L, 3, "malvado.legacy_sheet");
        lua_getfield(L, 2, "fpgIndex");
        lua_Integer index = lua_isnil(L, -1) ? -1 : luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        if (index < 0) {
            lua_pushinteger(L, 0);
            lua_setfield(L, 2, "width");
            lua_pushinteger(L, 0);
            lua_setfield(L, 2, "height");
            lua_settop(L, 0);
            return lua_yield(L, 0);
        }
        if (sheet->directory) {
            lua_Integer frame = index % sheet->count;
            lua_getiuservalue(L, 3, 1);
            lua_geti(L, -1, frame + 1);
            lua_remove(L, -2);
            LuaImage *image = luaL_checkudata(L, -1, "malvado.image");
            region_width = mv_image_width(image->image);
            region_height = mv_image_height(image->image);
        } else {
            int rows = sheet->rows, columns = sheet->columns;
            lua_getiuservalue(L, 3, 1);
            LuaImage *image = luaL_checkudata(L, -1, "malvado.image");
            region_width = mv_image_width(image->image) / columns;
            region_height = mv_image_height(image->image) / rows;
            lua_Integer frame = index % ((lua_Integer)rows * columns);
            x_source = (int)(frame % columns) * region_width;
            y_source = (int)(frame / columns) * region_height;
            sheet_image = 1;
        }
    } else {
        lua_getfield(L, 2, "graph");
        if (lua_isnil(L, -1)) {
            lua_settop(L, 0);
            return lua_yield(L, 0);
        }
        LuaImage *image = luaL_checkudata(L, -1, "malvado.image");
        region_width = mv_image_width(image->image);
        region_height = mv_image_height(image->image);
    }
    lua_Number x = legacy_number(L, 2, "x", 0);
    lua_Number y = legacy_number(L, 2, "y", 0);
    lua_Number angle = legacy_number(L, 2, "angle", 0);
    lua_Number scale = legacy_number(L, 2, "size", 1);
    if (scale <= 0 || scale > FLT_MAX)
        return luaL_error(L, "legacy graphic size must be positive and finite");
    lua_pushnumber(L, sheet_image ? 0 : region_width * scale);
    lua_setfield(L, 2, "width");
    lua_pushnumber(L, sheet_image ? 0 : region_height * scale);
    lua_setfield(L, 2, "height");
    lua_replace(L, 1);
    lua_settop(L, 1);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushinteger(L, x_source);
    lua_pushinteger(L, y_source);
    lua_pushinteger(L, region_width);
    lua_pushinteger(L, region_height);
    lua_pushnumber(L, sheet_image ? 0 : region_width / 2.0);
    lua_pushnumber(L, sheet_image ? 0 : region_height / 2.0);
    lua_pushnumber(L, scale);
    lua_pushnumber(L, angle);
    draw_image_region(L);
    lua_settop(L, 0);
    return lua_yield(L, 0);
}

#ifdef MALVADO_BUILD_ASSETS
static void destroy_managed_atlas(void *data) { mv_atlas_destroy(data); }

static int
atlas_gc(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    if (atlas->resource) mv_resource_release(atlas->resource);
    else mv_atlas_destroy(atlas->atlas);
    atlas->resource = NULL;
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
    *atlas = (LuaAtlas){0};
    luaL_setmetatable(L, "malvado.atlas");
    const char *error = NULL;
    atlas->atlas = mv_atlas_parse(json, length, mv_image_width(image->image),
                                  mv_image_height(image->image), &error);
    if (!atlas->atlas)
        return luaL_error(L, "%s", error);
    MvAtlas *native = atlas->atlas;
    atlas->atlas = NULL;
    atlas->resource = manage_native_resource(L, native, destroy_managed_atlas);
    atlas->atlas = native;
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    return 1;
}

static int
animation_frame(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    lua_Integer elapsed = luaL_checkinteger(L, 3);
    if (strlen(name) != length || elapsed < 0)
        return luaL_error(L, "invalid animation name or elapsed time");
    if (!lua_isnoneornil(L, 4) && !lua_isboolean(L, 4))
        return luaL_error(L, "loop must be a boolean");
    int loop = lua_isnoneornil(L, 4) ? 1 : lua_toboolean(L, 4);
    int id;
    if (!mv_atlas_animation_frame(atlas->atlas, name, (uint64_t)elapsed, loop, &id))
        return luaL_error(L, "unknown animation");
    lua_pushinteger(L, id);
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
font_gc(lua_State *L)
{
    LuaFont *font = luaL_checkudata(L, 1, "malvado.font");
    if (font->resource) mv_resource_release(font->resource);
    else mv_font_destroy(font->font);
    font->resource = NULL;
    font->font = NULL;
    return 0;
}

static int
load_font(lua_State *L)
{
    size_t length;
    const char *path = luaL_checklstring(L, 1, &length);
    lua_Integer size = luaL_checkinteger(L, 2);
    if (strlen(path) != length || size < 1 || size > 256)
        return luaL_error(L, "invalid font path or size");
    lua_Integer channels[4];
    for (int i = 0; i < 4; ++i) {
        channels[i] = luaL_optinteger(L, i + 3, 255);
        if (channels[i] < 0 || channels[i] > 255)
            return luaL_error(L, "color must be in 0..255");
    }
    uint32_t glyphs[1024];
    size_t count = 0;
    for (uint32_t codepoint = 32; codepoint <= 126; ++codepoint)
        glyphs[count++] = codepoint;
    for (uint32_t codepoint = 160; codepoint <= 255; ++codepoint)
        glyphs[count++] = codepoint;
    if (!lua_isnoneornil(L, 7)) {
        size_t extra_length;
        const char *extra = luaL_checklstring(L, 7, &extra_length);
        for (size_t offset = 0; offset < extra_length;) {
            uint32_t codepoint;
            size_t bytes = mv_utf8_decode(extra + offset, extra_length - offset, &codepoint);
            if (!bytes || codepoint == 0)
                return luaL_error(L, "invalid UTF-8 in font glyph list");
            offset += bytes;
            size_t i = 0;
            while (i < count && glyphs[i] != codepoint) ++i;
            if (i == count) {
                if (count == sizeof(glyphs) / sizeof(glyphs[0]))
                    return luaL_error(L, "too many font glyphs");
                glyphs[count++] = codepoint;
            }
        }
    }
    if (!engine.window)
        return luaL_error(L, "load_font requires an open window");
    LuaFont *font = lua_newuserdatauv(L, sizeof(*font), 0);
    *font = (LuaFont){0};
    luaL_setmetatable(L, "malvado.font");
    font->font = mv_font_load_codepoints(path, (int)size, glyphs, count);
    if (!font->font)
        return luaL_error(L, "could not load TTF or OTF font: %s", path);
    MvFont *native = font->font;
    font->font = NULL;
    font->resource = manage_native_resource(L, native, destroy_managed_font);
    font->font = native;
    lua_createtable(L, 4, 2);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "resource");
    lua_pushinteger(L, size);
    lua_setfield(L, -2, "size");
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, channels[i]);
        lua_seti(L, -2, i + 1);
    }
    return 1;
}

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
    size_t align_length;
    const char *align = luaL_optlstring(L, 5, "left", &align_length);
    if (strlen(align) != align_length ||
        (strcmp(align, "left") && strcmp(align, "center") && strcmp(align, "right")))
        return luaL_error(L, "alignment must be left, center, or right");
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_createtable(L, 0, 6);
    lua_pushstring(L, text);
    lua_setfield(L, -2, "text");
    lua_pushinteger(L, x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, y);
    lua_setfield(L, -2, "y");
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "font");
    lua_pushstring(L, align);
    lua_setfield(L, -2, "align");
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
    int has_x = !lua_isnoneornil(L, 3);
    int has_y = !lua_isnoneornil(L, 4);
    int has_font = !lua_isnoneornil(L, 5);
    lua_Integer x = has_x ? luaL_checkinteger(L, 3) : 0;
    lua_Integer y = has_y ? luaL_checkinteger(L, 4) : 0;
    if (has_font) {
        luaL_checktype(L, 5, LUA_TTABLE);
        lua_getfield(L, 5, "size");
        lua_Integer size = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        if (size < 1 || size > 1024)
            return luaL_error(L, "invalid font");
    }
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_geti(L, -1, id);
    if (lua_isnil(L, -1))
        return luaL_error(L, "unknown text id");
    lua_pushstring(L, text);
    lua_setfield(L, -2, "text");
    if (has_x) {
        lua_pushinteger(L, x);
        lua_setfield(L, -2, "x");
    }
    if (has_y) {
        lua_pushinteger(L, y);
        lua_setfield(L, -2, "y");
    }
    if (has_font) {
        lua_pushvalue(L, 5);
        lua_setfield(L, -2, "font");
    }
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
        lua_getfield(L, -1, "resource");
        LuaFont *custom = luaL_testudata(L, -1, "malvado.font");
        MvFont *resource = custom ? custom->font : NULL;
        lua_pop(L, 1);
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
        lua_getfield(L, -5, "align");
        const char *align = lua_tostring(L, -1);
        int width = resource ? mv_font_text_width(resource, text, size) : mv_window_text_width(text, size);
        int64_t offset = strcmp(align, "center") == 0 ? width / 2 :
                         strcmp(align, "right") == 0 ? width : 0;
        int64_t aligned_x = (int64_t)x - offset;
        if (aligned_x < INT_MIN) aligned_x = INT_MIN;
        if (aligned_x > INT_MAX) aligned_x = INT_MAX;
        lua_pop(L, 1);
        if (resource)
            (void)mv_font_draw(resource, text, (int)aligned_x, y, size, c);
        else
            mv_window_draw_text(text, (int)aligned_x, y, size, c);
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
    lua_Integer previous_closing_id = engine.closing_id;
    engine.closing_id = p->id;
    int status = lua_closethread(thread, engine.L);
    engine.closing_id = previous_closing_id;
    if (status != LUA_OK) {
        const char *message = lua_type(thread, -1) == LUA_TSTRING ?
                              lua_tostring(thread, -1) : "non-string error";
        fprintf(stderr, "Error closing process: %s\n", message);
        engine.failed = 1;
    }
    lua_rawgeti(engine.L, LUA_REGISTRYINDEX, self_ref);
    lua_pushnil(engine.L);
    lua_setiuservalue(engine.L, -2, 1);
    lua_pop(engine.L, 1);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, thread_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, self_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, draw_ref);
}

static int
lua_process_step_protected(lua_State *L)
{
    MvProcessId id = (MvProcessId)lua_tointeger(L, 1);
    /* The core calls only runnable processes. Synchronize all cached states
     * after the tick and after signals, rather than on every Lua resume. */
    Process *process = find_process((lua_Integer)id);
    if (!process) {
        lua_pushinteger(L, MV_STEP_ERROR);
        return 1;
    }
    int self_ref = process->self_ref;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    lua_getiuservalue(L, -1, 1);
    lua_pushliteral(L, "fps");
    lua_rawget(L, -2);
    lua_Integer fps = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 0;
    lua_pop(L, 3);
    process = find_process((lua_Integer)id);
    if (!process) {
        lua_pushinteger(L, MV_STEP_ERROR);
        return 1;
    }
    if (!process->first && mv_cadence_step(&process->cadence, (unsigned)fps) == 0) {
        lua_pushinteger(L, MV_STEP_YIELD);
        return 1;
    }
    lua_State *co = process->thread;
    int draw_ref = process->draw_ref;
    int first = process->first;
    process->delta = first ? 0 : (engine.tick_number - process->last_activation_tick) / 60.0;
    process->last_activation_tick = engine.tick_number;
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
    if (status != LUA_YIELD) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
        LuaSelf *self = lua_touserdata(L, -1);
        if (self)
            self->reason = status == LUA_OK ? 1 : 2;
        lua_pop(L, 1);
    }
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

/* Native processes do not have a Lua destructor. Clear their userdata fields
 * after scheduler cleanup without keeping those handles alive. */
static void
invalidate_dead_handles(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.process_handles");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        LuaSelf *self = luaL_testudata(L, -1, "malvado.process");
        MvProcessStatus status;
        if (self && mv_process_status(engine.context, (MvProcessId)self->id, &status) != MV_OK) {
            MvEndReason reason;
            if (mv_process_end_reason(engine.context, (MvProcessId)self->id, &reason))
                self->reason = (int)reason;
            lua_pushnil(L);
            lua_setiuservalue(L, -2, 1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static int
tick(void)
{
    engine.in_tick = 1;
    engine.tick_number++;
    MvResult result = mv_process_tick(engine.context);
    invalidate_dead_handles(engine.L);
    if (engine.audio) sweep_audio(engine.L);
    advance_fade();
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
render_primitive(lua_State *L, int command)
{
    int values[9];
    for (int i = 0; i < 9; ++i) {
        lua_geti(L, command, i + 1);
        values[i] = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    MvColor color = {values[5], values[6], values[7], values[8]};
    if (values[0] <= 2)
        (void)mv_window_draw_box(values[1], values[2], values[3], values[4], values[0] == 1, color);
    else
        (void)mv_window_draw_circle(values[1], values[2], values[3], values[0] == 3, color);
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

typedef struct {
    size_t slot;
    lua_Number z;
} DrawOrder;

static int
compare_draw_order(const void *a, const void *b)
{
    const DrawOrder *left = a;
    const DrawOrder *right = b;
    if (left->z < right->z) return -1;
    if (left->z > right->z) return 1;
    return (left->slot > right->slot) - (left->slot < right->slot);
}

static void
render(void)
{
    lua_State *L = engine.L;
    if (!lua_checkstack(L, 3)) {
        engine.failed = 1;
        return;
    }
    size_t count = engine.count;
    if (count > SIZE_MAX / sizeof(DrawOrder)) {
        engine.failed = 1;
        return;
    }
    DrawOrder *order = count ? malloc(count * sizeof(*order)) : NULL;
    if (count && !order) {
        fprintf(stderr, "Unable to allocate draw order\n");
        engine.failed = 1;
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, engine.items[i].self_ref);
        lua_getiuservalue(L, -1, 1);
        lua_pushliteral(L, "z");
        lua_rawget(L, -2);
        lua_Number z = lua_type(L, -1) == LUA_TNUMBER ? lua_tonumber(L, -1) : 0;
        order[i] = (DrawOrder){i, isfinite(z) ? z : 0};
        lua_pop(L, 3);
    }
    if (count > 1)
        qsort(order, count, sizeof(*order), compare_draw_order);
    mv_window_begin(engine.background);
    for (size_t p = 0; p < count; p++) {
        Process *process = &engine.items[order[p].slot];
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
            if (lua_isinteger(L, -3)) {
                render_primitive(L, command);
            } else if (image && lua_rawlen(L, command) == 11) {
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
    if (engine.fade.current.a != 0) {
        int width, height;
        if (mv_window_size(&width, &height))
            (void)mv_window_draw_box(0, 0, width, height, 1, engine.fade.current);
    }
    free(order);
    if (debug_frame_hook) {
        mv_window_flush();
        debug_frame_hook(debug_frame_user);
    }
    mv_window_end();
}

static void
validate_start_options(lua_State *L, int index, int allow_init)
{
    if (lua_getmetatable(L, index))
        luaL_error(L, "game table must not have a metatable");
    lua_pushnil(L);
    while (lua_next(L, index)) {
        if (lua_type(L, -2) != LUA_TSTRING)
            luaL_error(L, "game option names must be strings");
        size_t length;
        const char *key = lua_tolstring(L, -2, &length);
        if (strlen(key) != length)
            luaL_error(L, "invalid game option name");
        if (strcmp(key, "width") == 0 || strcmp(key, "height") == 0) {
            if (!lua_isinteger(L, -1) || lua_tointeger(L, -1) <= 0 ||
                lua_tointeger(L, -1) > INT_MAX)
                luaL_error(L, "invalid game dimension: %s", key);
        } else if (strcmp(key, "title") == 0) {
            size_t title_length;
            if (lua_type(L, -1) != LUA_TSTRING)
                luaL_error(L, "game title must be a string");
            const char *title = lua_tolstring(L, -1, &title_length);
            if (strlen(title) != title_length)
                luaL_error(L, "game title contains a zero byte");
        } else if (allow_init && strcmp(key, "init") == 0) {
            if (!lua_isfunction(L, -1))
                luaL_error(L, "game init must be a function");
        } else {
            luaL_error(L, "unknown game option: %s", key);
        }
        lua_pop(L, 1);
    }
}

static int
start(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (engine.started)
        return luaL_error(L, "start() can only be called once");
    if (!lua_isnoneornil(L, 2))
        luaL_checktype(L, 2, LUA_TTABLE);
    if (lua_istable(L, 2))
        validate_start_options(L, 2, 0);
    lua_Integer w = 800, h = 600;
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
    if (w <= 0 || h <= 0 || w > INT_MAX || h > INT_MAX)
        return luaL_error(L, "window dimensions must be positive integers in range");
    engine.started = 1;
    engine.width = w;
    engine.height = h;
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
empty_initializer(lua_State *L)
{
    (void)L;
    return 0;
}

static int
start_declared_game(lua_State *L)
{
    if (engine.started)
        return luaL_error(L, "game table returned after start()");
    validate_start_options(L, 1, 1);
    lua_pushcfunction(L, start);
    lua_pushliteral(L, "init");
    lua_rawget(L, 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushcfunction(L, empty_initializer);
    }
    lua_newtable(L);
    const char *options[] = {"width", "height", "title"};
    for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
        lua_pushstring(L, options[i]);
        lua_rawget(L, 1);
        lua_setfield(L, -2, options[i]);
    }
    lua_call(L, 2, 0);
    return 0;
}

static int
time_seconds(lua_State *L)
{
    double seconds;
    if (!mv_monotonic_time(&seconds))
        return luaL_error(L, "monotonic clock unavailable");
    lua_pushnumber(L, seconds);
    return 1;
}

static int
random_range(lua_State *L)
{
    lua_Integer minimum = luaL_checkinteger(L, 1);
    lua_Integer maximum = luaL_checkinteger(L, 2);
    if (minimum > maximum)
        return luaL_error(L, "minimum must not exceed maximum");
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushinteger(L, minimum);
    lua_pushinteger(L, maximum);
    lua_call(L, 2, 1);
    return 1;
}

static const struct { const char *name; const char *value; } api_constants[] = {
    {"KEY_A", "a"},
    {"KEY_B", "b"},
    {"KEY_C", "c"},
    {"KEY_D", "d"},
    {"KEY_E", "e"},
    {"KEY_F", "f"},
    {"KEY_G", "g"},
    {"KEY_H", "h"},
    {"KEY_I", "i"},
    {"KEY_J", "j"},
    {"KEY_K", "k"},
    {"KEY_L", "l"},
    {"KEY_M", "m"},
    {"KEY_N", "n"},
    {"KEY_O", "o"},
    {"KEY_P", "p"},
    {"KEY_Q", "q"},
    {"KEY_R", "r"},
    {"KEY_S", "s"},
    {"KEY_T", "t"},
    {"KEY_U", "u"},
    {"KEY_V", "v"},
    {"KEY_W", "w"},
    {"KEY_X", "x"},
    {"KEY_Y", "y"},
    {"KEY_Z", "z"},
    {"KEY_0", "0"},
    {"KEY_1", "1"},
    {"KEY_2", "2"},
    {"KEY_3", "3"},
    {"KEY_4", "4"},
    {"KEY_5", "5"},
    {"KEY_6", "6"},
    {"KEY_7", "7"},
    {"KEY_8", "8"},
    {"KEY_9", "9"},
    {"KEY_ESCAPE", "escape"},
    {"KEY_SPACE", "space"},
    {"KEY_LEFT", "left"},
    {"KEY_RIGHT", "right"},
    {"KEY_UP", "up"},
    {"KEY_DOWN", "down"},
    {"KEY_ENTER", "enter"},
    {"KEY_TAB", "tab"},
    {"KEY_BACKSPACE", "backspace"},
    {"SIGNAL_SLEEP", "sleep"},
    {"SIGNAL_FREEZE", "freeze"},
    {"SIGNAL_WAKEUP", "wakeup"},
    {"SIGNAL_KILL", "kill"},
};

static const char *const global_names[] = {
    "process",
    "spawn",
    "find",
    "children",
    "descendants",
    "snapshot",
    "frame",
    "signal",
    "signal_tree",
    "set_priority",
    "get_priority",
    "kill",
    "alive",
    "kill_tree",
    "status",
    "send",
    "start",
    "set_title",
    "window_size",
    "set_color",
    "fade_to",
    "fade_active",
    "fade_color",
    "time_seconds",
    "clear_screen",
    "key",
    "key_down",
    "mouse_position",
    "mouse_button",
    "pad_connected",
    "pad_button",
    "pad_axis",
    "text_input_next",
    "load_sound",
    "clone_sound",
    "play_sound",
    "stop_sound",
    "pause_sound",
    "resume_sound",
    "sound_playing",
    "sound_volume",
    "load_music",
    "music_playing",
    "music_volume",
    "draw_text",
    "load_image",
    "image_size",
    "draw_image",
    "draw_image_region",
    "load_atlas",
    "animation_frame",
    "draw_sprite",
    "font",
    "load_font",
    "write",
    "change_text",
    "delete_text",
    "fade_off",
    "fade_on",
    "draw_box",
    "draw_rect",
    "draw_fcircle",
    "draw_circle",
    "rand",
    "play_music",
    "update_music",
    "stop_music",
    "pause_music",
    "resume_music",
    "get_screen_width",
    "get_screen_height",
    "KEY_A",
    "KEY_B",
    "KEY_C",
    "KEY_D",
    "KEY_E",
    "KEY_F",
    "KEY_G",
    "KEY_H",
    "KEY_I",
    "KEY_J",
    "KEY_K",
    "KEY_L",
    "KEY_M",
    "KEY_N",
    "KEY_O",
    "KEY_P",
    "KEY_Q",
    "KEY_R",
    "KEY_S",
    "KEY_T",
    "KEY_U",
    "KEY_V",
    "KEY_W",
    "KEY_X",
    "KEY_Y",
    "KEY_Z",
    "KEY_0",
    "KEY_1",
    "KEY_2",
    "KEY_3",
    "KEY_4",
    "KEY_5",
    "KEY_6",
    "KEY_7",
    "KEY_8",
    "KEY_9",
    "KEY_ESCAPE",
    "KEY_SPACE",
    "KEY_LEFT",
    "KEY_RIGHT",
    "KEY_UP",
    "KEY_DOWN",
    "KEY_ENTER",
    "KEY_TAB",
    "KEY_BACKSPACE",
    "SIGNAL_SLEEP",
    "SIGNAL_FREEZE",
    "SIGNAL_WAKEUP",
    "SIGNAL_KILL",
    NULL
};


static int
spawn_process(lua_State *L)
{
    if (!lua_iscfunction(L, 1) || lua_tocfunction(L, 1) != create_process)
        return luaL_error(L, "expected a process constructor");
    lua_settop(L, 3);
    lua_call(L, 2, 1);
    return 1;
}

static int
call_process_namespace(lua_State *L)
{
    lua_remove(L, 1);
    return define_process(L);
}

static int
open_module(lua_State *L)
{
    static const luaL_Reg functions[] = {
        {"process", define_process},
        {"spawn", spawn_process},
        {"find", find_by_type},
        {"children", process_children},
        {"descendants", process_descendants},
        {"snapshot", snapshot},
        {"frame", frame},
        {"signal", signal_process},
        {"signal_tree", signal_tree},
        {"set_priority", set_priority},
        {"get_priority", get_priority},
        {"kill", kill_process},
        {"alive", process_alive},
        {"kill_tree", kill_tree},
        {"status", process_status},
        {"send", send_message},
        {"start", start},
        {"set_title", set_title},
        {"window_size", window_size},
        {"set_color", set_color},
        {"fade_to", fade_to},
        {"fade_active", fade_active},
        {"fade_color", fade_color},
        {"time_seconds", time_seconds},
        {"clear_screen", clear_screen},
        {"key", key},
        {"mouse_position", mouse_position},
        {"mouse_button", mouse_button},
        {"pad_connected", pad_connected},
        {"pad_button", pad_button},
        {"pad_axis", pad_axis},
        {"text_input_next", text_input_next},
        {"load_sound", load_sound},
        {"clone_sound", clone_sound},
        {"play_sound", play_sound},
        {"stop_sound", stop_sound},
        {"pause_sound", pause_sound},
        {"resume_sound", resume_sound},
        {"sound_playing", sound_playing},
        {"sound_volume", sound_volume},
        {"load_music", load_music},
        {"music_playing", music_playing},
        {"music_volume", music_volume},
        {"draw_text", text},
        {"load_image", load_image},
        {"image_size", image_size},
        {"draw_image", draw_image},
        {"draw_image_region", draw_image_region},
        {"legacy_fpg", legacy_fpg},
#ifdef MALVADO_BUILD_ASSETS
        {"load_atlas", load_atlas},
        {"animation_frame", animation_frame},
        {"draw_sprite", draw_sprite},
#endif
        {"font", font},
        {"load_font", load_font},
        {"write", write_text},
        {"change_text", change_text},
        {"delete_text", delete_text},
        {NULL, NULL}
    };
    for (size_t i = 0; global_names[i]; ++i) {
        for (size_t j = i + 1; global_names[j]; ++j) {
            if (strcmp(global_names[i], global_names[j]) == 0)
                return luaL_error(L, "duplicate global export: %s", global_names[i]);
        }
    }
    for (size_t i = 0; i < sizeof(api_constants) / sizeof(api_constants[0]); ++i) {
        for (size_t j = i + 1; j < sizeof(api_constants) / sizeof(api_constants[0]); ++j) {
            if (strcmp(api_constants[i].name, api_constants[j].name) == 0)
                return luaL_error(L, "duplicate constant export: %s", api_constants[i].name);
        }
    }
    for (size_t i = 0; functions[i].name; ++i) {
        for (size_t j = i + 1; functions[j].name; ++j) {
            if (strcmp(functions[i].name, functions[j].name) == 0)
                return luaL_error(L, "duplicate module export: %s", functions[i].name);
        }
    }
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
    luaL_newmetatable(L, "malvado.font");
    lua_pushcfunction(L, font_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado font");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.legacy_sheet");
    lua_pushliteral(L, "malvado legacy sheet");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
#ifndef _WIN32
    luaL_newmetatable(L, "malvado.directory_entries");
    lua_pushcfunction(L, directory_entries_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado directory entries");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
#endif
    luaL_newmetatable(L, "malvado.sound");
    lua_pushcfunction(L, sound_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado sound");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.music");
    lua_pushcfunction(L, music_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "malvado music");
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
    lua_newtable(L);
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.process_handles");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.playing_audio");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    luaL_newlib(L, functions);
    for (size_t i = 0; i < sizeof(api_constants) / sizeof(api_constants[0]); ++i) {
        lua_pushstring(L, api_constants[i].value);
        lua_setfield(L, -2, api_constants[i].name);
    }
    lua_newtable(L);
    lua_getfield(L, -2, "process");
    lua_setfield(L, -2, "define");
    static const char *const process_functions[] = {
        "spawn", "find", "children", "descendants", "snapshot", "frame", "signal", "signal_tree", "set_priority",
        "get_priority", "kill", "alive", "kill_tree", "status", "send"
    };
    for (size_t i = 0; i < sizeof(process_functions) / sizeof(process_functions[0]); ++i) {
        lua_getfield(L, -2, process_functions[i]);
        lua_setfield(L, -2, process_functions[i]);
    }
    for (size_t i = 0; i < sizeof(api_constants) / sizeof(api_constants[0]); ++i) {
        if (strncmp(api_constants[i].name, "SIGNAL_", 7) == 0) {
            lua_pushstring(L, api_constants[i].value);
            lua_setfield(L, -2, api_constants[i].name);
        }
    }
    lua_newtable(L);
    lua_pushcfunction(L, call_process_namespace);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "process");
    lua_getfield(L, -1, "key");
    lua_setfield(L, -2, "key_down");
    lua_newtable(L);
    static const char *const input_functions[] = {
        "key_down", "mouse_position", "mouse_button", "pad_connected", "pad_button",
        "pad_axis", "text_input_next"
    };
    for (size_t i = 0; i < sizeof(input_functions) / sizeof(input_functions[0]); ++i) {
        lua_getfield(L, -2, input_functions[i]);
        lua_setfield(L, -2, input_functions[i]);
    }
    for (size_t i = 0; i < sizeof(api_constants) / sizeof(api_constants[0]); ++i) {
        if (strncmp(api_constants[i].name, "KEY_", 4) == 0) {
            lua_pushstring(L, api_constants[i].value);
            lua_setfield(L, -2, api_constants[i].name);
        }
    }
    lua_setfield(L, -2, "input");
    lua_pushboolean(L, 1);
    lua_pushcclosure(L, fade_switch, 1);
    lua_setfield(L, -2, "fade_off");
    lua_pushboolean(L, 0);
    lua_pushcclosure(L, fade_switch, 1);
    lua_setfield(L, -2, "fade_on");
    static const char *const primitives[] = {"draw_box", "draw_rect", "draw_fcircle", "draw_circle"};
    for (size_t i = 0; i < sizeof(primitives) / sizeof(primitives[0]); ++i) {
        lua_pushinteger(L, (lua_Integer)i + 1);
        lua_pushcclosure(L, draw_primitive, 1);
        lua_setfield(L, -2, primitives[i]);
    }
    lua_getglobal(L, "math");
    lua_getfield(L, -1, "random");
    lua_remove(L, -2);
    lua_pushcclosure(L, random_range, 1);
    lua_setfield(L, -2, "rand");
    static const char *const music_actions[] = {
        "play_music", "update_music", "stop_music", "pause_music", "resume_music"
    };
    for (size_t i = 0; i < sizeof(music_actions) / sizeof(music_actions[0]); ++i) {
        lua_pushinteger(L, (lua_Integer)i);
        lua_pushcclosure(L, music_action, 1);
        lua_setfield(L, -2, music_actions[i]);
    }
    lua_pushinteger(L, 0);
    lua_pushcclosure(L, legacy_screen_dimension, 1);
    lua_setfield(L, -2, "get_screen_width");
    lua_pushinteger(L, 1);
    lua_pushcclosure(L, legacy_screen_dimension, 1);
    lua_setfield(L, -2, "get_screen_height");
    lua_pushvalue(L, -1);
    lua_pushlightuserdata(L, (void *)global_names);
    lua_pushcclosure(L, malvado_import_globals, 2);
    lua_setfield(L, -2, "import_globals");
    return 1;
}

static const char *const legacy_names[] = {
    "process", "frame", "font", "write", "change_text", "delete_text",
    "key", "clear_screen", "set_title", "get_screen_width", "get_screen_height",
    "load_image", "time_seconds", "rand", "fpg", "set_color", "draw_box",
    "draw_rect", "draw_fcircle", "draw_circle", "fade_off", "fade_on", NULL
};

static void
legacy_global_value(lua_State *L, const char *name)
{
    if (strcmp(name, "frame") == 0)
        lua_pushvalue(L, lua_upvalueindex(2));
    else
        lua_getfield(L, lua_upvalueindex(1), strcmp(name, "fpg") == 0 ? "legacy_fpg" : name);
}

static int
legacy_import_globals(lua_State *L)
{
    for (size_t i = 0; legacy_names[i]; ++i) {
        lua_getglobal(L, legacy_names[i]);
        legacy_global_value(L, legacy_names[i]);
        int allowed = lua_isnil(L, -2) || lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        if (!allowed)
            return luaL_error(L, "global already exists: %s", legacy_names[i]);
    }
    for (size_t i = 0; legacy_names[i]; ++i) {
        legacy_global_value(L, legacy_names[i]);
        lua_setglobal(L, legacy_names[i]);
    }
    return 0;
}

static int
open_legacy_module(lua_State *L)
{
    lua_newtable(L);
    luaL_requiref(L, "malvado", open_module, 0);
    lua_pushcfunction(L, legacy_frame);
    lua_pushcclosure(L, legacy_import_globals, 2);
    lua_setfield(L, -2, "import_globals");
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
    engine.draw_color = (MvColor){255, 255, 255, 255};
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
    luaL_requiref(engine.L, "malvado.compat.legacy", open_legacy_module, 0);
    lua_pop(engine.L, 1);
    if (luaL_loadfile(engine.L, argv[index]) != LUA_OK || lua_pcall(engine.L, 0, 1, 0) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(engine.L, -1));
        engine.failed = 1;
    } else if (lua_istable(engine.L, -1)) {
        lua_pushcfunction(engine.L, start_declared_game);
        lua_pushvalue(engine.L, -2);
        if (lua_pcall(engine.L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "%s\n", lua_tostring(engine.L, -1));
            engine.failed = 1;
        }
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
    if (engine.audio)
        mv_audio_close();
    if (engine.window)
        mv_window_close();
    return engine.failed ? 1 : 0;
}
