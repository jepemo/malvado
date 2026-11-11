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
#include "malvado/camera.h"
#include "malvado/animation.h"
#include "malvado/collision.h"
#ifdef MALVADO_BUILD_ASSETS
#include "malvado/atlas.h"
#include "malvado/fpg.h"
#include "cJSON.h"
#endif
#include "globals.h"
#include "core/clock.h"
#include "core/process.h"
#include "core/schedule.h"
#include "malvado/malvado.h"
#include "malvado/graphic.h"
#include "runner/runtime.h"

typedef enum {
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_FROZEN,
    PROCESS_KILLED
} ProcessState;

typedef struct { MvCamera value; int closed, target_ref; } LuaCamera;

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
    int legacy_frame;
    int alive;
} Process;
typedef struct {
    lua_State *L;
    MvContext *context;
    MvRuntimeId runtime_id;
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
    LuaCamera main_camera;
    MvDrawState draw;
    MvFadeQueue fade;
} Engine;
static Engine engine;
static MalvadoDebugHook debug_hook;
static void *debug_user;
static MalvadoDebugHook debug_frame_hook;
static void *debug_frame_user;
static MvStepResult lua_process_step(MvContext *ctx, MvProcessId id, void *user);
static void lua_process_destroy(void *user);
static void update_cameras(lua_State *L);

typedef struct { lua_Integer id; } LuaProcessHandle;
typedef struct {
    lua_Integer id, parent;
    int reason, collision_invalid;
    MvProcessRef reference;
    MvAnimationState animation_state;
    lua_Integer animation_tick;
} LuaSelf;
typedef struct { int x, y, width, height; } LuaRegion;
typedef struct { float x, y; } LuaScroll;
typedef struct { MvImage *image; MvResource *resource; double pivot_x, pivot_y; } LuaImage;
typedef struct {
    MvCollisionShape shape;
    MvColor color;
    int top_left, sprite;
    MvImageRegion region;
    int pivot_x, pivot_y;
} LuaGraphic;
typedef struct { MvAnimation *sequence; MvCollisionShape shape; } LuaAnimation;
typedef struct { MvSound *sound; MvResource *resource; int playing, paused; } LuaSound;
typedef struct { MvMusic *music; MvResource *resource; int playing, paused; } LuaMusic;
typedef struct { MvFont *font; MvResource *resource; } LuaFont;
typedef struct { int rows, columns, count, directory; } LuaLegacySheet;
#ifndef _WIN32
typedef struct { char **names; size_t count; } LuaDirectoryEntries;
#endif
#ifdef MALVADO_BUILD_ASSETS
typedef struct { MvAtlas *atlas; MvResource *resource; } LuaAtlas;
typedef struct { MvFpg *fpg; int transparent_zero; } LuaFpg;
typedef struct { MvFpgManifest *manifest; } LuaFpgManifest;
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
validate_options_extra(lua_State *L, int index, const char *first, const char *second, const char *third)
{
    index = lua_absindex(L, index);
    luaL_checktype(L, index, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, index)) {
        size_t length = 0;
        const char *name = lua_type(L, -2) == LUA_TSTRING ? lua_tolstring(L, -2, &length) : NULL;
        if (!name || strlen(name) != length ||
            (strcmp(name, first) != 0 && (!second || strcmp(name, second) != 0) &&
             (!third || strcmp(name, third) != 0)))
            luaL_error(L, "unknown process option");
        lua_pop(L, 1);
    }
}

static void
validate_options(lua_State *L, int index, const char *first, const char *second)
{
    validate_options_extra(L, index, first, second, NULL);
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

/* Numeric visual properties live in C. Lua keeps number presentation and invalid
 * visual overrides. Invalid values retain the existing hide/error behavior. */
static void
refresh_process_transform(lua_State *L, LuaSelf *self, int fields)
{
    fields = lua_absindex(L, fields);
    double x, y;
    if (mv_process_position(engine.context, self->reference, &x, &y) != MV_OK)
        return;
    double angle, scale;
    if (mv_process_angle(engine.context, self->reference, &angle) != MV_OK ||
        mv_process_scale(engine.context, self->reference, &scale) != MV_OK)
        return;
    const char *names[] = {"x", "y", "angle", "size"};
    double values[] = {x, y, angle, scale};
    for (unsigned i = 0; i < 4; ++i) {
        raw_option(L, fields, names[i]);
        int empty = lua_isnil(L, -1);
        int numeric = lua_type(L, -1) == LUA_TNUMBER && isfinite(lua_tonumber(L, -1));
        double fallback = i == 3 ? 1 : 0;
        int matches = (empty && values[i] == fallback) ||
                      (numeric && lua_tonumber(L, -1) == values[i]);
        lua_pop(L, 1);
        if ((empty || numeric) && !matches) {
            lua_pushstring(L, names[i]);
            if (values[i] == 0) lua_pushinteger(L, 0);
            else lua_pushnumber(L, values[i]);
            lua_rawset(L, fields);
        }
    }
}

static void
set_process_property(lua_State *L, LuaSelf *self, const char *name, int value)
{
    int empty = lua_isnil(L, value);
    if (!empty && (lua_type(L, value) != LUA_TNUMBER || !isfinite(lua_tonumber(L, value))))
        return;
    double x, y;
    if (mv_process_position(engine.context, self->reference, &x, &y) != MV_OK)
        luaL_error(L, "process is no longer alive");
    double coordinate = empty ? (!strcmp(name, "size") ? 1 : 0) : lua_tonumber(L, value);
    if (!strcmp(name, "angle")) {
        (void)mv_process_set_angle(engine.context, self->reference, coordinate);
        return;
    }
    if (!strcmp(name, "size")) {
        (void)mv_process_set_scale(engine.context, self->reference, coordinate);
        return;
    }
    if (!strcmp(name, "x")) x = coordinate;
    else y = coordinate;
    if (mv_process_set_position(engine.context, self->reference, x, y) != MV_OK)
        luaL_error(L, "cannot set process position");
}

static int
self_index(lua_State *L)
{
    LuaSelf *self = luaL_checkudata(L, 1, "malvado.process");
    if (engine.context) {
        (void)check_process_id(L, 1);
    } else if (!engine.L || !self->reference.id ||
               self->reference.runtime.high != engine.runtime_id.high ||
               self->reference.runtime.low != engine.runtime_id.low ||
               self->reference.runtime.sequence != engine.runtime_id.sequence) {
        return luaL_error(L, "process handle belongs to another runtime");
    }
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
    if (key && (!strcmp(key, "x") || !strcmp(key, "y") ||
                !strcmp(key, "angle") || !strcmp(key, "size"))) {
        lua_getiuservalue(L, 1, 1);
        refresh_process_transform(L, self, -1);
        raw_option(L, -1, key);
        if (lua_isnil(L, -1)) lua_pushinteger(L, !strcmp(key, "size") ? 1 : 0);
        return 1;
    }
    if (key && strcmp(key, "z") == 0) {
        double z;
        if (mv_process_z(engine.context, self->reference, &z) != MV_OK)
            return luaL_error(L, "process is no longer alive");
        /* Keep Lua integer/float readback while its native rendering value
         * matches. The table entry is presentation data, never the sort key. */
        lua_getiuservalue(L, 1, 1);
        raw_option(L, -1, "z");
        if (lua_type(L, -1) == LUA_TNUMBER && lua_tonumber(L, -1) == z)
            return 1;
        if (z == 0) lua_pushinteger(L, 0);
        else lua_pushnumber(L, z);
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
collider_number(lua_State *L, int fields, const char *name, double fallback, double *out)
{
    raw_option(L, fields, name);
    int valid = lua_isnil(L, -1) || lua_isnumber(L, -1);
    *out = lua_isnil(L, -1) ? fallback : lua_tonumber(L, -1);
    lua_pop(L, 1);
    return valid && isfinite(*out);
}

/* Native visual properties supply valid numbers. Invalid Lua fields remove the native
 * collider until corrected; collision queries report the invalid state. */
static void
sync_process_collider(lua_State *L, LuaSelf *self, int fields)
{
    MvProcessRef reference = self->reference;
    fields = lua_absindex(L, fields);
    refresh_process_transform(L, self, fields);
    MvCollisionShape shape = {0};
    int present = 0, invalid = 0;
    Process *slot = find_process_slot((lua_Integer)reference.id);
    int legacy = slot && slot->legacy_frame;
    raw_option(L, fields, "collider");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        if (legacy) {
            raw_option(L, fields, "fpg");
            invalid = !lua_isnil(L, -1);
            lua_pop(L, 1);
        }
        raw_option(L, fields, "graph");
        if (!invalid && !lua_isnil(L, -1)) {
            LuaGraphic *graphic = luaL_testudata(L, -1, "malvado.graphic");
            LuaImage *image = luaL_testudata(L, -1, "malvado.image");
            LuaAnimation *animation = luaL_testudata(L, -1, "malvado.animation");
            if (animation) {
                shape = animation->shape;
                present = 1;
            } else if (graphic) {
                shape = graphic->shape;
                present = 1;
            } else if (image && image->image) {
                int width = mv_image_width(image->image), height = mv_image_height(image->image);
                if (width > 0 && height > 0) {
                    shape = (MvCollisionShape){.kind = MV_COLLISION_RECT, .width = width, .height = height,
                        .offset_x = legacy ? 0 : width / 2.0 - image->pivot_x,
                        .offset_y = legacy ? 0 : height / 2.0 - image->pivot_y};
                    present = 1;
                } else invalid = 1;
            } else invalid = 1;
        }
    } else if (!(lua_isboolean(L, -1) && !lua_toboolean(L, -1))) {
        MvCollisionShape *explicit_shape = luaL_testudata(L, -1, "malvado.collision_shape");
        if (explicit_shape) {
            shape = *explicit_shape;
            present = 1;
        } else invalid = 1;
    }
    lua_pop(L, 1);
    MvCollisionTransform transform = {0, 0, 0, 1};
    if (present && (!collider_number(L, fields, "x", 0, &transform.x) ||
                    !collider_number(L, fields, "y", 0, &transform.y) ||
                    !collider_number(L, fields, "angle", 0, &transform.angle) ||
                    !collider_number(L, fields, "size", 1, &transform.scale)))
        invalid = 1;
    if (present && !invalid &&
        mv_process_collider_set(engine.context, reference, shape, transform) != MV_OK)
        invalid = 1;
    if (!present || invalid)
        (void)mv_process_collider_clear(engine.context, reference);
    self->collision_invalid = invalid;
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
    if (key && strcmp(key, "camera") == 0 && !lua_isnil(L, 3) &&
        !(lua_isboolean(L, 3) && !lua_toboolean(L, 3))) {
        LuaCamera *camera = luaL_testudata(L, 3, "malvado.camera");
        if (!camera || camera->closed)
            return luaL_error(L, "camera must be a live camera, nil, or false");
    }
    if (key && strcmp(key, "scroll") == 0 && !lua_isnil(L, 3) &&
        !luaL_testudata(L, 3, "malvado.scroll"))
        return luaL_error(L, "scroll must be a scroll descriptor or nil");
    if (key && strcmp(key, "region") == 0 && !lua_isnil(L, 3) &&
        !luaL_testudata(L, 3, "malvado.region"))
        return luaL_error(L, "region must be a region descriptor or nil");
    if (key && strcmp(key, "z") == 0 &&
        (lua_type(L, 3) != LUA_TNUMBER || !isfinite(lua_tonumber(L, 3))))
        return luaL_error(L, "z must be a finite number");
    if (key && strcmp(key, "children_attached") == 0 && lua_type(L, 3) != LUA_TBOOLEAN)
        return luaL_error(L, "children_attached must be a boolean");
    if (key && strcmp(key, "fps") == 0 &&
        (!lua_isinteger(L, 3) || lua_tointeger(L, 3) < 0 || lua_tointeger(L, 3) > 60))
        return luaL_error(L, "fps must be an integer in 0..60");
    if (key && strcmp(key, "z") == 0) {
        if (mv_process_set_z(engine.context, self->reference, lua_tonumber(L, 3)) != MV_OK)
            return luaL_error(L, "process is no longer alive");
        lua_getiuservalue(L, 1, 1);
        lua_pushliteral(L, "z");
        lua_pushvalue(L, 3);
        lua_rawset(L, -3);
        return 0;
    }
    if (key && (!strcmp(key, "x") || !strcmp(key, "y") ||
                !strcmp(key, "angle") || !strcmp(key, "size")))
        set_process_property(L, self, key, 3);
    lua_getiuservalue(L, 1, 1);
    int graph_changed = 0;
    if (key && !strcmp(key, "graph")) {
        raw_option(L, -1, "graph");
        graph_changed = !lua_rawequal(L, -1, 3);
        lua_pop(L, 1);
    }
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    if (graph_changed) {
        self->animation_state = (MvAnimationState){0};
        self->animation_tick = engine.tick_number;
    }
    if (key && (!strcmp(key, "x") || !strcmp(key, "y") || !strcmp(key, "angle") ||
                !strcmp(key, "size") || !strcmp(key, "graph") || !strcmp(key, "collider") ||
                !strcmp(key, "fpg")))
        sync_process_collider(L, self, -1);
    return 0;
}

/* Copy ordinary Lua tables; share the supported native resource references. */
static void
copy_default(lua_State *L, int source, int copies, unsigned depth)
{
    source = lua_absindex(L, source);
    copies = lua_absindex(L, copies);
    if (depth > 256 || !lua_checkstack(L, 8))
        luaL_error(L, "defaults are too deeply nested");
    int type = lua_type(L, source);
    if (type == LUA_TUSERDATA &&
        (luaL_testudata(L, source, "malvado.collision_shape") ||
         luaL_testudata(L, source, "malvado.graphic") ||
         luaL_testudata(L, source, "malvado.animation") ||
         luaL_testudata(L, source, "malvado.region") ||
         luaL_testudata(L, source, "malvado.scroll") ||
         luaL_testudata(L, source, "malvado.image") ||
         luaL_testudata(L, source, "malvado.camera") ||
         luaL_testudata(L, source, "malvado.collision_world")
#ifdef MALVADO_BUILD_ASSETS
         || luaL_testudata(L, source, "malvado.atlas")
         || luaL_testudata(L, source, "malvado.fpg")
         || luaL_testudata(L, source, "malvado.fpg_manifest")
#endif
         )) {
        lua_pushvalue(L, source);
        return;
    }
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

typedef struct {
    LuaSelf *self;
    lua_State *thread;
    int self_ref, thread_ref, draw_ref;
} SpawnDraft;

/* All Lua allocations for a process happen before the native spawn. */
static int
prepare_spawn(lua_State *L)
{
    SpawnDraft *draft = lua_touserdata(L, 1);
    draft->self = lua_newuserdatauv(L, sizeof(*draft->self), 1);
    *draft->self = (LuaSelf){0};
    lua_pushvalue(L, 2);
    lua_setiuservalue(L, -2, 1);
    luaL_getmetatable(L, "malvado.process");
    lua_setmetatable(L, -2);
    draft->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    draft->thread = lua_newthread(L);
    draft->thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (!lua_checkstack(draft->thread, 2))
        return luaL_error(L, "cannot prepare process coroutine");
    lua_pushvalue(L, 3);
    lua_xmove(L, draft->thread, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, draft->self_ref);
    lua_xmove(L, draft->thread, 1);
    lua_newtable(L);
    draft->draw_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

static void
discard_spawn(lua_State *L, SpawnDraft *draft)
{
    luaL_unref(L, LUA_REGISTRYINDEX, draft->draw_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, draft->thread_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, draft->self_ref);
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
    lua_pushliteral(L, "camera");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) && !(lua_isboolean(L, -1) && !lua_toboolean(L, -1))) {
        LuaCamera *camera = luaL_testudata(L, -1, "malvado.camera");
        if (!camera || camera->closed)
            return luaL_error(L, "camera must be a live camera, nil, or false");
    }
    lua_pop(L, 1);
    lua_pushliteral(L, "scroll");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) && !luaL_testudata(L, -1, "malvado.scroll"))
        return luaL_error(L, "scroll must be a scroll descriptor or nil");
    lua_pop(L, 1);
    lua_pushliteral(L, "region");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) && !luaL_testudata(L, -1, "malvado.region"))
        return luaL_error(L, "region must be a region descriptor or nil");
    lua_pop(L, 1);
    lua_pushliteral(L, "fps");
    lua_rawget(L, fields);
    if (!lua_isnil(L, -1) &&
        (!lua_isinteger(L, -1) || lua_tointeger(L, -1) < 0 || lua_tointeger(L, -1) > 60))
        return luaL_error(L, "fps must be an integer in 0..60");
    lua_pop(L, 1);
    lua_Integer creator = engine.closing_id ? engine.closing_id : engine.active_id;
    if (!creator && explicit_attachment && attached)
        return luaL_error(L, "cannot attach a process without an owner");
    if (engine.closing_id && (!explicit_attachment || attached))
        return luaL_error(L, "a terminating process can only spawn independent processes explicitly");
    if (creator && !explicit_attachment && !engine.closing_id) {
        Process *owner = find_process(creator);
        if (!owner)
            return luaL_error(L, "process creator is no longer alive");
        lua_rawgeti(L, LUA_REGISTRYINDEX, owner->self_ref);
        lua_getiuservalue(L, -1, 1);
        lua_getfield(L, -1, "children_attached");
        attached = lua_toboolean(L, -1);
        lua_pop(L, 3);
    }
    MvProcessId parent = attached ? (MvProcessId)creator : 0;
    if (!lua_checkstack(L, 12))
        return luaL_error(L, "cannot prepare process");
    SpawnDraft draft = {.self_ref = LUA_NOREF, .thread_ref = LUA_NOREF, .draw_ref = LUA_NOREF};
    lua_pushcfunction(L, prepare_spawn);
    lua_pushlightuserdata(L, &draft);
    lua_pushvalue(L, fields);
    lua_pushvalue(L, lua_upvalueindex(1));
    if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
        discard_spawn(L, &draft);
        return lua_error(L);
    }

    /* Finalizers can create processes during preparation, so check capacity now. */
    if (engine.count == engine.capacity) {
        if (engine.capacity > SIZE_MAX / 2 / sizeof(*engine.items)) {
            discard_spawn(L, &draft);
            return luaL_error(L, "too many processes");
        }
        size_t n = engine.capacity ? engine.capacity * 2 : 16;
        Process *items = realloc(engine.items, n * sizeof(*items));
        if (!items) {
            discard_spawn(L, &draft);
            return luaL_error(L, "out of memory creating process");
        }
        engine.items = items;
        engine.capacity = n;
    }
    MvProcessId next_id;
    if (mv_process_next_id(engine.context, &next_id) != MV_OK || next_id > LUA_MAXINTEGER) {
        discard_spawn(L, &draft);
        return luaL_error(L, "process ID space exhausted");
    }
    LuaProcessHandle *handle = malloc(sizeof(*handle));
    if (!handle) {
        discard_spawn(L, &draft);
        return luaL_error(L, "out of memory creating process");
    }
    MvProcessId core_id;
    MvResult spawn_result = mv_process_spawn_related(engine.context, (MvProcessId)creator, parent,
                                                     0, lua_process_step, lua_process_destroy,
                                                     handle, &core_id);
    if (spawn_result != MV_OK) {
        free(handle);
        discard_spawn(L, &draft);
        return luaL_error(L, "failed to create process");
    }
    handle->id = (lua_Integer)core_id;
    (void)mv_process_set_type(engine.context, core_id, (uint64_t)lua_tointeger(L, lua_upvalueindex(2)));
    engine.next_process_id = handle->id;
    *draft.self = (LuaSelf){.id = handle->id, .parent = creator, .animation_tick = engine.tick_number,
                            .reference = {engine.runtime_id, core_id}};
    engine.items[engine.count++] = (Process) {
        .id = handle->id,
        .type_id = lua_tointeger(L, lua_upvalueindex(2)),
        .thread = draft.thread,
        .thread_ref = draft.thread_ref,
        .self_ref = draft.self_ref,
        .draw_ref = draft.draw_ref,
        .parent_id = (lua_Integer)parent,
        .state = PROCESS_RUNNING,
        .first = 1,
        .alive = 1
    };
    raw_option(L, fields, "z");
    if (!lua_isnil(L, -1))
        (void)mv_process_set_z(engine.context, draft.self->reference, lua_tonumber(L, -1));
    lua_pop(L, 1);
    raw_option(L, fields, "x");
    set_process_property(L, draft.self, "x", -1);
    lua_pop(L, 1);
    raw_option(L, fields, "y");
    set_process_property(L, draft.self, "y", -1);
    lua_pop(L, 1);
    raw_option(L, fields, "angle");
    set_process_property(L, draft.self, "angle", -1);
    lua_pop(L, 1);
    raw_option(L, fields, "size");
    set_process_property(L, draft.self, "size", -1);
    lua_pop(L, 1);
    sync_process_collider(L, draft.self, fields);
    lua_rawgeti(L, LUA_REGISTRYINDEX, draft.self_ref);
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
    Process *process = find_process_slot((lua_Integer)info->id);
    if (process) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
        return;
    }
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
    MvProcessInfo *items = NULL;
    size_t total = 0;
    int captured = 0;
    for (int attempt = 0; attempt < 16 && !captured; ++attempt) {
        if (count > SIZE_MAX / sizeof(*items))
            return luaL_error(L, "too many processes");
        items = lua_newuserdatauv(L, count * sizeof(*items), 0);
        if (mv_process_find(engine.context, (uint64_t)type_id, items, count, &total) != MV_OK)
            return luaL_error(L, "failed to inspect processes");
        if (total > count) {
            lua_pop(L, 1);
            count = total;
        } else {
            captured = 1;
        }
    }
    if (!captured) return luaL_error(L, "process list changed during inspection");
    lua_newtable(L);
    lua_Integer result = 0;
    for (size_t i = 0; i < count && i < total; i++) {
        if (items[i].id <= LUA_MAXINTEGER) {
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
    MvProcessInfo *items = NULL;
    size_t total = 0;
    int captured = 0;
    for (int attempt = 0; attempt < 16 && !captured; ++attempt) {
        if (count > SIZE_MAX / sizeof(*items))
            return luaL_error(L, "too many processes");
        items = lua_newuserdatauv(L, count * sizeof(*items), 0);
        MvResult result;
        if (creation && direct)
            result = mv_process_creation_children(engine.context, (MvProcessId)root, type, items, count, &total);
        else if (creation)
            result = mv_process_creation_descendants(engine.context, (MvProcessId)root, type, items, count, &total);
        else if (direct)
            result = mv_process_children(engine.context, (MvProcessId)root, type, items, count, &total);
        else
            result = mv_process_descendants(engine.context, (MvProcessId)root, type, items, count, &total);
        if (result != MV_OK) return luaL_error(L, "invalid process query root");
        if (total > count) {
            lua_pop(L, 1);
            count = total;
        } else {
            captured = 1;
        }
    }
    if (!captured) return luaL_error(L, "process list changed during inspection");
    lua_newtable(L);
    lua_Integer output = 0;
    for (size_t i = 0; i < total && i < count; ++i) {
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
    MvInspectionHeader header;
    MvResult probe = mv_process_inspection_capture(engine.context, &header, NULL, 0, &count,
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
        MvResult result = mv_process_inspection_capture(engine.context, &header, items, count, &total,
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
    if (header.tick > LUA_MAXINTEGER)
        return luaL_error(L, "inspection tick is too large");
    for (size_t i = 0; i < total; ++i) {
        Process *process = find_process_slot((lua_Integer)items[i].id);
        int draw_ref = process ? process->draw_ref : LUA_NOREF;
        lua_rawgeti(L, LUA_REGISTRYINDEX, draw_ref);
        draw_counts[i] = lua_rawlen(L, -1);
        lua_pop(L, 1);
    }
    MvRuntimeId runtime_id = header.runtime;
    char runtime_text[49];
    (void)snprintf(runtime_text, sizeof(runtime_text), "%016" PRIx64 "%016" PRIx64 "%016" PRIx64,
                   runtime_id.high, runtime_id.low, runtime_id.sequence);
    lua_newtable(L);
    lua_pushstring(L, runtime_text);
    lua_setfield(L, -2, "runtime");
    lua_pushinteger(L, (lua_Integer)header.tick);
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
    Process *process = find_process(engine.active_id);
    if (process && process->legacy_frame) {
        process->legacy_frame = 0;
        lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
        LuaSelf *self = lua_touserdata(L, -1);
        lua_getiuservalue(L, -1, 1);
        sync_process_collider(L, self, -1);
        lua_pop(L, 2);
    }
    return lua_yield(L, 0);
}

static int
queue_fade(lua_State *L, MvColor color, unsigned ticks)
{
    if (mv_fade_queue_push(&engine.fade, color, ticks) != 1)
        return luaL_error(L, "fade queue is full");
    return 0;
}

static void
advance_fade(void)
{
    (void)mv_fade_queue_step(&engine.fade);
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
    lua_pushboolean(L, mv_fade_queue_active(&engine.fade));
    return 1;
}

static int
fade_color(lua_State *L)
{
    MvColor color = engine.fade.fade.current;
    lua_pushinteger(L, color.r);
    lua_pushinteger(L, color.g);
    lua_pushinteger(L, color.b);
    lua_pushinteger(L, color.a);
    return 4;
}

static int
set_color(lua_State *L)
{
    lua_Integer values[4] = {0, 0, 0, 255};
    MvDrawState next = engine.draw;
    for (int i = 0; i < 4; ++i) {
        values[i] = i == 3 ? luaL_optinteger(L, i + 1, 255) : luaL_checkinteger(L, i + 1);
        if (!mv_draw_state_color(&next, values[0], values[1], values[2], values[3]))
            return luaL_error(L, "color must be in 0..255");
    }
    engine.draw = next;
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
        engine.draw.color.r, engine.draw.color.g, engine.draw.color.b, engine.draw.color.a
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
    lua_Integer r = luaL_checkinteger(L, 1);
    lua_Integer g = luaL_checkinteger(L, 2);
    lua_Integer b = luaL_checkinteger(L, 3);
    if (!mv_draw_state_background(&engine.draw, r, g, b))
        return luaL_error(L, "color must be in 0..255");
    return 0;
}

static int
query_key(lua_State *L, int (*query)(MvKey))
{
    size_t length;
    const char *s = luaL_checklstring(L, 1, &length);
    MvKey parsed;
    if (strlen(s) != length || !mv_key_from_name(s, &parsed))
        return luaL_error(L, "unsupported key: %s", s);
    lua_pushboolean(L, !engine.headless && query(parsed));
    return 1;
}

static int
key(lua_State *L)
{
    return query_key(L, mv_key_down);
}
static int
key_pressed(lua_State *L)
{
    return query_key(L, mv_key_pressed);
}
static int
key_released(lua_State *L)
{
    return query_key(L, mv_key_released);
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
    lua_Integer x = luaL_checkinteger(L, 2);
    lua_Integer y = luaL_checkinteger(L, 3);
    if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX)
        return luaL_error(L, "image coordinate is outside the supported range");
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
    process->legacy_frame = 1;
    int self_ref = process->self_ref;
    lua_settop(L, 0);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    lua_getiuservalue(L, 1, 1);
    sync_process_collider(L, lua_touserdata(L, 1), 2);
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
static int
fpg_gc(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    mv_fpg_destroy(value->fpg);
    value->fpg = NULL;
    return 0;
}

static int
fpg_parse(lua_State *L)
{
    size_t length;
    const char *data = luaL_checklstring(L, 1, &length);
    LuaFpg *value = lua_newuserdatauv(L, sizeof(*value), 1);
    value->fpg = NULL;
    value->transparent_zero = 1;
    luaL_setmetatable(L, "malvado.fpg");
    const char *error = NULL;
    value->fpg = mv_fpg_parse(data, length, &error);
    if (!value->fpg) return luaL_error(L, "%s", error);
    return 1;
}

static int
fpg_manifest_gc(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    mv_fpg_manifest_destroy(value->manifest);
    value->manifest = NULL;
    return 0;
}

static int
fpg_manifest_parse(lua_State *L)
{
    size_t length;
    const char *json = luaL_checklstring(L, 1, &length);
    LuaFpgManifest *value = lua_newuserdatauv(L, sizeof(*value), 2);
    value->manifest = NULL;
    luaL_setmetatable(L, "malvado.fpg_manifest");
    const char *error = NULL;
    value->manifest = mv_fpg_manifest_parse(json, length, &error);
    if (!value->manifest) return luaL_error(L, "%s", error);
    return 1;
}

static int
fpg_manifest_count(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_pushinteger(L, (lua_Integer)mv_fpg_manifest_count(value->manifest));
    return 1;
}

static int
fpg_manifest_id_at(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_Integer index = luaL_checkinteger(L, 2);
    MvFpgManifestSprite sprite;
    if (index < 1 || (uint64_t)(index - 1) > SIZE_MAX ||
        !mv_fpg_manifest_at(value->manifest, (size_t)(index - 1), &sprite))
        lua_pushnil(L);
    else lua_pushinteger(L, sprite.id);
    return 1;
}

static int
fpg_manifest_transparent(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_pushboolean(L, mv_fpg_manifest_transparent_zero(value->manifest));
    return 1;
}

static int
fpg_manifest_palette(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_pushlstring(L, (const char *)mv_fpg_manifest_palette(value->manifest), 768);
    return 1;
}

static int
fpg_manifest_gamma(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_pushlstring(L, (const char *)mv_fpg_manifest_gamma(value->manifest), 576);
    return 1;
}

static int
fpg_manifest_sprite(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_Integer id = luaL_checkinteger(L, 2);
    MvFpgManifestSprite sprite;
    if (id < 1 || id > 999 || !mv_fpg_manifest_sprite(value->manifest, (int)id, &sprite)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 7);
    lua_pushinteger(L, sprite.id); lua_setfield(L, -2, "id");
    lua_pushinteger(L, sprite.width); lua_setfield(L, -2, "width");
    lua_pushinteger(L, sprite.height); lua_setfield(L, -2, "height");
    lua_pushinteger(L, (lua_Integer)sprite.point_count); lua_setfield(L, -2, "point_count");
    lua_pushfstring(L, "%d.png", sprite.id); lua_setfield(L, -2, "image");
    lua_pushlstring(L, (const char *)sprite.name, sizeof(sprite.name)); lua_setfield(L, -2, "name_raw");
    lua_pushlstring(L, (const char *)sprite.filename, sizeof(sprite.filename)); lua_setfield(L, -2, "filename_raw");
    return 1;
}

static int
fpg_manifest_point(lua_State *L)
{
    LuaFpgManifest *value = luaL_checkudata(L, 1, "malvado.fpg_manifest");
    lua_Integer id = luaL_checkinteger(L, 2), index = luaL_checkinteger(L, 3);
    MvFpgPoint point;
    if (id < 1 || id > 999 || index < 0 || (uint64_t)index > SIZE_MAX ||
        !mv_fpg_manifest_point(value->manifest, (int)id, (size_t)index, &point)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, point.x); lua_setfield(L, -2, "x");
    lua_pushinteger(L, point.y); lua_setfield(L, -2, "y");
    lua_pushboolean(L, point.defined); lua_setfield(L, -2, "defined");
    return 1;
}

static int
fpg_count(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_pushinteger(L, (lua_Integer)mv_fpg_count(value->fpg));
    return 1;
}

static int
fpg_id_at(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer index = luaL_checkinteger(L, 2);
    MvFpgSprite sprite;
    if (index < 1 || (uint64_t)(index - 1) > SIZE_MAX ||
        !mv_fpg_at(value->fpg, (size_t)(index - 1), &sprite))
        lua_pushnil(L);
    else
        lua_pushinteger(L, sprite.id);
    return 1;
}

static int
fpg_palette(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_pushlstring(L, (const char *)mv_fpg_palette(value->fpg), 768);
    return 1;
}

static int
fpg_gamma(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_pushlstring(L, (const char *)mv_fpg_gamma(value->fpg), 576);
    return 1;
}

static int
fpg_sprite(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer id = luaL_checkinteger(L, 2);
    MvFpgSprite sprite;
    if (id < 1 || id > 999 || !mv_fpg_sprite(value->fpg, (int)id, &sprite)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 7);
    lua_pushinteger(L, sprite.id);
    lua_setfield(L, -2, "id");
    lua_pushinteger(L, sprite.width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, sprite.height);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, (lua_Integer)sprite.point_count);
    lua_setfield(L, -2, "point_count");
    lua_pushlstring(L, (const char *)sprite.name, sizeof(sprite.name));
    lua_setfield(L, -2, "name_raw");
    lua_pushlstring(L, (const char *)sprite.filename, sizeof(sprite.filename));
    lua_setfield(L, -2, "filename_raw");
    lua_pushlstring(L, (const char *)sprite.pixels, (size_t)sprite.width * sprite.height);
    lua_setfield(L, -2, "pixels");
    return 1;
}

static int
fpg_point(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer id = luaL_checkinteger(L, 2), index = luaL_checkinteger(L, 3);
    MvFpgPoint point;
    if (id < 1 || id > 999 || index < 0 || (uint64_t)index > SIZE_MAX ||
        !mv_fpg_point(value->fpg, (int)id, (size_t)index, &point)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, point.x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, point.y);
    lua_setfield(L, -2, "y");
    lua_pushboolean(L, point.defined);
    lua_setfield(L, -2, "defined");
    return 1;
}

static int
fpg_rgba(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer id = luaL_checkinteger(L, 2);
    if (!lua_isnoneornil(L, 3) && !lua_isboolean(L, 3))
        return luaL_error(L, "transparent_zero must be a boolean");
    MvFpgSprite sprite;
    if (id < 1 || id > 999 || !mv_fpg_sprite(value->fpg, (int)id, &sprite)) {
        lua_pushnil(L);
        return 1;
    }
    int transparent_zero = lua_toboolean(L, 3);
    size_t length = (size_t)sprite.width * (size_t)sprite.height * 4;
    luaL_Buffer buffer;
    char *output = luaL_buffinitsize(L, &buffer, length);
    if (!mv_fpg_copy_rgba(value->fpg, (int)id, transparent_zero, (uint8_t *)output, length))
        return luaL_error(L, "cannot convert FPG sprite");
    luaL_pushresultsize(&buffer, length);
    return 1;
}

static int
fpg_json(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    if (!lua_isnoneornil(L, 2) && !lua_isboolean(L, 2))
        return luaL_error(L, "transparent_zero must be a boolean");
    int transparent_zero = lua_toboolean(L, 2);
    size_t length = mv_fpg_json_size(value->fpg, transparent_zero);
    if (!length) return luaL_error(L, "cannot encode FPG metadata");
    luaL_Buffer buffer;
    char *output = luaL_buffinitsize(L, &buffer, length);
    if (!mv_fpg_copy_json(value->fpg, transparent_zero, output, length))
        return luaL_error(L, "cannot encode FPG metadata");
    luaL_pushresultsize(&buffer, length - 1);
    return 1;
}

static int
fpg_png(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer id = luaL_checkinteger(L, 2);
    if (!lua_isnoneornil(L, 3) && !lua_isboolean(L, 3))
        return luaL_error(L, "transparent_zero must be a boolean");
    if (id < 1 || id > 999) {
        lua_pushnil(L);
        return 1;
    }
    int transparent_zero = lua_toboolean(L, 3);
    size_t length = mv_fpg_png_size(value->fpg, (int)id, transparent_zero);
    if (!length) {
        lua_pushnil(L);
        return 1;
    }
    luaL_Buffer buffer;
    char *output = luaL_buffinitsize(L, &buffer, length);
    if (!mv_fpg_copy_png(value->fpg, (int)id, transparent_zero, (uint8_t *)output, length))
        return luaL_error(L, "cannot encode FPG sprite");
    luaL_pushresultsize(&buffer, length);
    return 1;
}

static int
fpg_image(lua_State *L)
{
    LuaFpg *value = luaL_checkudata(L, 1, "malvado.fpg");
    lua_Integer id = luaL_checkinteger(L, 2);
    if (!lua_isnoneornil(L, 3) && !lua_isboolean(L, 3))
        return luaL_error(L, "transparent_zero must be a boolean");
    MvFpgSprite sprite;
    if (id < 1 || id > 999 || !mv_fpg_sprite(value->fpg, (int)id, &sprite)) {
        lua_pushnil(L);
        return 1;
    }
    if (!engine.window)
        return luaL_error(L, "FPG image requires an open window");
    int transparent_zero = lua_toboolean(L, 3);
    LuaImage *image = lua_newuserdatauv(L, sizeof(*image), 0);
    *image = (LuaImage){0};
    luaL_setmetatable(L, "malvado.image");
    size_t length = (size_t)sprite.width * (size_t)sprite.height * 4;
    luaL_Buffer buffer;
    char *rgba = luaL_buffinitsize(L, &buffer, length);
    if (!mv_fpg_copy_rgba(value->fpg, (int)id, transparent_zero, (uint8_t *)rgba, length))
        return luaL_error(L, "cannot convert FPG sprite");
    image->image = mv_image_from_rgba(sprite.width, sprite.height, (uint8_t *)rgba, length);
    luaL_pushresultsize(&buffer, length);
    lua_pop(L, 1);
    if (!image->image)
        return luaL_error(L, "could not create FPG image");
    manage_image(L, image);
    return 1;
}

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
load_atlas_format(lua_State *L, int aseprite)
{
    LuaImage *image = luaL_checkudata(L, 1, "malvado.image");
    size_t length;
    const char *json = luaL_checklstring(L, 2, &length);
    LuaAtlas *atlas = lua_newuserdatauv(L, sizeof(*atlas), 2);
    *atlas = (LuaAtlas){0};
    luaL_setmetatable(L, "malvado.atlas");
    const char *error = NULL;
    if (aseprite)
        atlas->atlas = mv_atlas_parse_aseprite(json, length, mv_image_width(image->image),
                                               mv_image_height(image->image), &error);
    else
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
load_atlas(lua_State *L)
{
    return load_atlas_format(L, 0);
}

static int
load_aseprite(lua_State *L)
{
    return load_atlas_format(L, 1);
}

static int
atlas_pivot_color(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    MvColor color;
    if (!mv_atlas_pivot_color(atlas->atlas, &color)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 4, 0);
    const int channels[] = {color.r, color.g, color.b, color.a};
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, channels[i]);
        lua_seti(L, -2, i + 1);
    }
    return 1;
}

static int
animation_repeats(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    if (strlen(name) != length)
        return luaL_error(L, "invalid animation name");
    unsigned repeats;
    if (mv_atlas_animation_repeats(atlas->atlas, name, &repeats))
        lua_pushinteger(L, repeats);
    else
        lua_pushnil(L);
    return 1;
}

static int
animation_color(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    if (strlen(name) != length)
        return luaL_error(L, "invalid animation name");
    MvColor color;
    if (!mv_atlas_animation_color(atlas->atlas, name, &color)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 4, 0);
    const int channels[] = {color.r, color.g, color.b, color.a};
    for (int i = 0; i < 4; ++i) {
        lua_pushinteger(L, channels[i]);
        lua_seti(L, -2, i + 1);
    }
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
sprite_id(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    size_t length;
    const char *name = luaL_checklstring(L, 2, &length);
    if (strlen(name) != length)
        return luaL_error(L, "invalid sprite name");
    MvAtlasSprite sprite;
    if (mv_atlas_find_name(atlas->atlas, name, &sprite))
        lua_pushinteger(L, sprite.id);
    else
        lua_pushnil(L);
    return 1;
}

/* Return a snapshot; it does not retain the atlas or expose borrowed pointers. */
static int
sprite_info(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    lua_Integer id = luaL_checkinteger(L, 2);
    MvAtlasSprite sprite;
    if (id <= 0 || id > INT_MAX || !mv_atlas_find(atlas->atlas, (int)id, &sprite))
        return luaL_error(L, "unknown sprite id");
    lua_createtable(L, 0, 12);
    lua_pushstring(L, sprite.name);
    lua_setfield(L, -2, "name");
    const char *names[] = {"id", "x", "y", "width", "height", "pivot_x", "pivot_y",
                           "source_width", "source_height", "trim_x", "trim_y"};
    const int values[] = {sprite.id, sprite.region.x, sprite.region.y,
                          sprite.region.width, sprite.region.height, sprite.pivot_x,
                          sprite.pivot_y, sprite.source_width, sprite.source_height,
                          sprite.trim_x, sprite.trim_y};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        lua_pushinteger(L, values[i]);
        lua_setfield(L, -2, names[i]);
    }
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

/* Read a bounded source without keeping a FILE open across Lua allocations. */
static const char *
read_asset_file(lua_State *L, const char *path, size_t *length)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        luaL_error(L, "cannot open asset: %s", path);
    int seek_ok = fseek(file, 0, SEEK_END) == 0;
    long size = seek_ok ? ftell(file) : -1;
    int closed = fclose(file);
    if (size < 0 || size > 64 * 1024 * 1024 || closed != 0)
        luaL_error(L, "cannot read asset (maximum 64 MiB): %s", path);
    char *bytes = lua_newuserdatauv(L, (size_t)size + 1, 0);
    file = fopen(path, "rb");
    if (!file)
        luaL_error(L, "cannot reopen asset: %s", path);
    size_t count = fread(bytes, 1, (size_t)size, file);
    int extra = fgetc(file);
    int failed = ferror(file);
    closed = fclose(file);
    if (count != (size_t)size || extra != EOF || failed || closed != 0)
        luaL_error(L, "cannot read asset or source changed: %s", path);
    bytes[count] = '\0';
    *length = count;
    return bytes;
}

#ifdef MALVADO_BUILD_ASSETS
static void
push_asset_path(lua_State *L, const char *source, const char *image)
{
    const char *slash = strrchr(source, '/');
    int absolute = image[0] == '/';
#ifdef _WIN32
    const char *backslash = strrchr(source, '\\');
    if (backslash && (!slash || backslash > slash))
        slash = backslash;
    absolute = absolute || image[0] == '\\' || (image[0] && image[1] == ':');
#endif
    if (absolute || !slash) {
        lua_pushstring(L, image);
        return;
    }
    lua_pushlstring(L, source, (size_t)(slash - source) + 1);
    lua_pushstring(L, image);
    lua_concat(L, 2);
}

static void
push_asset_image(lua_State *L, const char *source, const char *image)
{
    lua_pushcfunction(L, load_image);
    push_asset_path(L, source, image);
    lua_call(L, 1, 1);
}

typedef enum { ASSET_UNKNOWN, ASSET_PNG, ASSET_FPG, ASSET_ATLAS,
               ASSET_ASEPRITE, ASSET_MANIFEST } AssetFormat;

/* Routing only: the existing native importer validates the complete schema
 * before any referenced image is opened. Never infer a JSON dialect by suffix. */
static AssetFormat
asset_json_format(const char *bytes, size_t length, int *width, int *height)
{
    if (memchr(bytes, '\0', length) || length > 16 * 1024 * 1024)
        return ASSET_UNKNOWN;
    cJSON *root = cJSON_ParseWithLengthOpts(bytes, length + 1, NULL, 1);
    if (!root)
        return ASSET_UNKNOWN;
    const cJSON *kind = cJSON_GetObjectItemCaseSensitive(root, "kind");
    const cJSON *meta = cJSON_GetObjectItemCaseSensitive(root, "meta");
    const cJSON *frames = cJSON_GetObjectItemCaseSensitive(root, "frames");
    AssetFormat format = ASSET_UNKNOWN;
    if (cJSON_IsString(kind) && !meta && !frames) {
        if (!strcmp(kind->valuestring, "graphics-library"))
            format = ASSET_ATLAS;
        else if (!strcmp(kind->valuestring, "malvado-fpg-8"))
            format = ASSET_MANIFEST;
    } else if (!kind && meta && frames) {
        format = ASSET_ASEPRITE;
        const cJSON *size = cJSON_GetObjectItemCaseSensitive(meta, "size");
        const cJSON *w = cJSON_GetObjectItemCaseSensitive(size, "w");
        const cJSON *h = cJSON_GetObjectItemCaseSensitive(size, "h");
        if (cJSON_IsNumber(w) && cJSON_IsNumber(h) &&
            w->valuedouble >= 1 && w->valuedouble <= INT_MAX &&
            h->valuedouble >= 1 && h->valuedouble <= INT_MAX) {
            *width = (int)w->valuedouble;
            *height = (int)h->valuedouble;
        }
    }
    cJSON_Delete(root);
    return format;
}

static int
load_asset_atlas(lua_State *L, const char *path, const char *bytes, size_t length,
                 int aseprite, int width, int height)
{
    LuaAtlas *atlas = lua_newuserdatauv(L, sizeof(*atlas), 2);
    *atlas = (LuaAtlas){0};
    luaL_setmetatable(L, "malvado.atlas");
    int index = lua_gettop(L);
    const char *error = NULL;
    atlas->atlas = aseprite ? mv_atlas_parse_aseprite(bytes, length, width, height, &error) :
                             mv_atlas_parse(bytes, length, width, height, &error);
    if (!atlas->atlas)
        return luaL_error(L, "%s: %s", path, error);
    if (engine.window) {
        push_asset_image(L, path, mv_atlas_image(atlas->atlas));
        LuaImage *image = lua_touserdata(L, -1);
        MvAtlas *validated = aseprite ?
            mv_atlas_parse_aseprite(bytes, length, mv_image_width(image->image),
                                   mv_image_height(image->image), &error) :
            mv_atlas_parse(bytes, length, mv_image_width(image->image),
                           mv_image_height(image->image), &error);
        if (!validated)
            return luaL_error(L, "%s: %s", path, error);
        mv_atlas_destroy(atlas->atlas);
        atlas->atlas = validated;
        lua_setiuservalue(L, index, 1);
    }
    MvAtlas *native = atlas->atlas;
    atlas->atlas = NULL;
    atlas->resource = manage_native_resource(L, native, destroy_managed_atlas);
    atlas->atlas = native;
    return 1;
}

static int
load_asset_library(lua_State *L, const char *path, const char *bytes, size_t length,
                   AssetFormat format, int transparent)
{
    lua_pushcfunction(L, format == ASSET_FPG ? fpg_parse : fpg_manifest_parse);
    lua_pushlstring(L, bytes, length);
    lua_call(L, 1, 1);
    int library = lua_gettop(L);
    lua_newtable(L);
    int images = lua_gettop(L);
    if (format == ASSET_FPG) {
        LuaFpg *fpg = lua_touserdata(L, library);
        fpg->transparent_zero = transparent;
        if (engine.window) {
            for (size_t i = 0; i < mv_fpg_count(fpg->fpg); ++i) {
                MvFpgSprite sprite;
                (void)mv_fpg_at(fpg->fpg, i, &sprite);
                lua_pushcfunction(L, fpg_image);
                lua_pushvalue(L, library);
                lua_pushinteger(L, sprite.id);
                lua_pushboolean(L, transparent);
                lua_call(L, 3, 1);
                lua_rawseti(L, images, sprite.id);
            }
        }
        lua_setiuservalue(L, library, 1);
    } else {
        LuaFpgManifest *manifest = lua_touserdata(L, library);
        if (engine.window) {
            for (size_t i = 0; i < mv_fpg_manifest_count(manifest->manifest); ++i) {
                MvFpgManifestSprite sprite;
                (void)mv_fpg_manifest_at(manifest->manifest, i, &sprite);
                char name[32];
                (void)snprintf(name, sizeof(name), "%d.png", sprite.id);
                push_asset_image(L, path, name);
                LuaImage *image = lua_touserdata(L, -1);
                if (mv_image_width(image->image) != sprite.width ||
                    mv_image_height(image->image) != sprite.height)
                    return luaL_error(L, "converted FPG image dimensions do not match: %d", sprite.id);
                lua_rawseti(L, images, sprite.id);
            }
        }
        lua_setiuservalue(L, library, 2);
        lua_pushstring(L, path);
        lua_setiuservalue(L, library, 1);
    }
    return 1;
}
#endif

static int
load_asset(lua_State *L)
{
    size_t path_length;
    const char *path = luaL_checklstring(L, 1, &path_length);
    if (!path_length || strlen(path) != path_length)
        return luaL_error(L, "invalid asset path");
    const char *requested = NULL;
    int transparent = 1, has_transparent = 0, has_pivot = 0, centered = 0;
    if (!lua_isnoneornil(L, 2)) {
        validate_options_extra(L, 2, "format", "transparent_zero", "pivot");
        raw_option(L, 2, "format");
        if (!lua_isnil(L, -1)) {
            size_t length;
            requested = luaL_checklstring(L, -1, &length);
            if (strlen(requested) != length ||
                (strcmp(requested, "png") && strcmp(requested, "fpg") &&
                 strcmp(requested, "atlas") && strcmp(requested, "aseprite") &&
                 strcmp(requested, "fpg-manifest")))
                return luaL_error(L, "unknown asset format");
        }
        /* Keep the option value rooted until loading completes. */
        raw_option(L, 2, "pivot");
        if (!lua_isnil(L, -1)) {
            size_t length;
            const char *pivot = luaL_checklstring(L, -1, &length);
            if (strlen(pivot) != length || (strcmp(pivot, "center") && strcmp(pivot, "top-left")))
                return luaL_error(L, "PNG pivot must be center or top-left");
            has_pivot = 1;
            centered = !strcmp(pivot, "center");
        }
        lua_pop(L, 1);
        raw_option(L, 2, "transparent_zero");
        if (!lua_isnil(L, -1)) {
            if (!lua_isboolean(L, -1))
                return luaL_error(L, "transparent_zero must be a boolean");
            transparent = lua_toboolean(L, -1);
            has_transparent = 1;
        }
        lua_pop(L, 1);
    }
    size_t length;
    const char *bytes = read_asset_file(L, path, &length);
    int png = length >= 8 && !memcmp(bytes, "\x89PNG\r\n\x1a\n", 8);
    if ((requested && !strcmp(requested, "png")) || (!requested && png)) {
        if (has_transparent)
            return luaL_error(L, "transparent_zero is only valid for FPG assets");
        lua_pushcfunction(L, load_image);
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
        if (centered) {
            LuaImage *image = lua_touserdata(L, -1);
            image->pivot_x = mv_image_width(image->image) / 2.0;
            image->pivot_y = mv_image_height(image->image) / 2.0;
        }
        return 1;
    }
    if (has_pivot) return luaL_error(L, "pivot option is only valid for PNG assets");
#ifdef MALVADO_BUILD_ASSETS
    int width = INT_MAX, height = INT_MAX;
    AssetFormat detected = length >= 4 && !memcmp(bytes, "fpg\x1a", 4) ? ASSET_FPG :
                           asset_json_format(bytes, length, &width, &height);
    AssetFormat format = detected;
    if (requested) {
        if (!strcmp(requested, "fpg")) format = ASSET_FPG;
        else if (!strcmp(requested, "atlas")) format = ASSET_ATLAS;
        else if (!strcmp(requested, "aseprite")) format = ASSET_ASEPRITE;
        else format = ASSET_MANIFEST;
    }
    if (has_transparent && format != ASSET_FPG)
        return luaL_error(L, "transparent_zero is only valid for FPG assets");
    if (format == ASSET_ATLAS || format == ASSET_ASEPRITE)
        return load_asset_atlas(L, path, bytes, length, format == ASSET_ASEPRITE, width, height);
    if (format == ASSET_FPG || format == ASSET_MANIFEST)
        return load_asset_library(L, path, bytes, length, format, transparent);
    return luaL_error(L, "unknown or ambiguous asset schema/format: %s", path);
#else
    (void)transparent;
    return luaL_error(L, "asset format requires asset support (only PNG is available)");
#endif
}


static int
text(lua_State *L)
{
    size_t text_length;
    const char *s = luaL_checklstring(L, 1, &text_length);
    if (strlen(s) != text_length)
        return luaL_error(L, "text contains a zero byte");
    lua_Integer x = luaL_checkinteger(L, 2);
    lua_Integer y = luaL_checkinteger(L, 3);
    lua_Integer size = luaL_optinteger(L, 4, 20);
    if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX ||
        size < INT_MIN || size > INT_MAX)
        return luaL_error(L, "text coordinate or size is outside the supported range");
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
    if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX)
        return luaL_error(L, "text coordinate is outside the supported range");
    size_t text_length;
    const char *text = luaL_checklstring(L, 4, &text_length);
    if (strlen(text) != text_length)
        return luaL_error(L, "text contains a zero byte");
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
    lua_pushinteger(L, engine.active ? engine.active_id : 0);
    lua_setfield(L, -2, "owner");
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
    size_t text_length;
    const char *text = luaL_checklstring(L, 2, &text_length);
    if (strlen(text) != text_length)
        return luaL_error(L, "text contains a zero byte");
    int has_x = !lua_isnoneornil(L, 3);
    int has_y = !lua_isnoneornil(L, 4);
    int has_font = !lua_isnoneornil(L, 5);
    lua_Integer x = has_x ? luaL_checkinteger(L, 3) : 0;
    lua_Integer y = has_y ? luaL_checkinteger(L, 4) : 0;
    if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX)
        return luaL_error(L, "text coordinate is outside the supported range");
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
release_process_texts(lua_State *L, lua_Integer owner)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_getfield(L, -1, "owner");
        int owned = lua_tointeger(L, -1) == owner;
        lua_pop(L, 1);
        lua_pop(L, 1);
        if (owned) {
            lua_pushvalue(L, -1);
            lua_pushnil(L);
            lua_rawset(L, -4);
        }
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
    lua_Integer id = p->id;
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
    release_process_texts(engine.L, id);
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
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }
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

static void
advance_animations(lua_State *L)
{
    if (!lua_checkstack(L, 8)) { engine.failed = 1; return; }
    for (size_t i = 0; i < engine.count; ++i) {
        Process *process = &engine.items[i];
        if (!process->alive || process->state != PROCESS_RUNNING) continue;
        lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
        LuaSelf *self = lua_touserdata(L, -1);
        lua_getiuservalue(L, -1, 1);
        raw_option(L, -1, "graph");
        LuaAnimation *animation = luaL_testudata(L, -1, "malvado.animation");
        if (animation && self->animation_tick < engine.tick_number) {
            if (!mv_animation_advance(animation->sequence, &self->animation_state, 60))
                engine.failed = 1;
        }
        self->animation_tick = engine.tick_number;
        lua_pop(L, 3);
    }
}

static int
tick(void)
{
    if (engine.window)
        mv_input_tick();
    engine.in_tick = 1;
    engine.tick_number++;
    MvResult result = mv_process_tick(engine.context);
    invalidate_dead_handles(engine.L);
    if (engine.audio) sweep_audio(engine.L);
    advance_fade();
    sync_processes();
    advance_animations(engine.L);
    update_cameras(engine.L);
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

static int
render_number(lua_State *L, int fields, const char *name, double fallback, double *value)
{
    raw_option(L, fields, name);
    int valid = lua_isnil(L, -1) || lua_type(L, -1) == LUA_TNUMBER;
    *value = lua_isnil(L, -1) ? fallback : lua_tonumber(L, -1);
    lua_pop(L, 1);
    return valid && isfinite(*value) && fabs(*value) <= FLT_MAX / 16;
}

static void
render_graph(lua_State *L, const Process *process)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
    lua_getiuservalue(L, -1, 1);
    int fields = lua_gettop(L);
    refresh_process_transform(L, lua_touserdata(L, -2), fields);
    raw_option(L, fields, "graph");
    LuaAnimation *animation = luaL_testudata(L, -1, "malvado.animation");
    if (animation) {
        LuaSelf *self = lua_touserdata(L, -3);
        MvAnimationSample sample;
        if (mv_animation_sample(animation->sequence, self->animation_state.elapsed_us, &sample)) {
            lua_getiuservalue(L, -1, 1);
            lua_rawgeti(L, -1, (lua_Integer)sample.index + 1);
            lua_remove(L, -2);
            lua_replace(L, -2);
        }
    }
    LuaGraphic *graphic = luaL_testudata(L, -1, "malvado.graphic");
    LuaImage *image = luaL_testudata(L, -1, "malvado.image");
    double x, y, angle, scale;
    if ((graphic || (image && !process->legacy_frame && !process->first)) &&
        render_number(L, fields, "x", 0, &x) &&
        render_number(L, fields, "y", 0, &y) &&
        render_number(L, fields, "angle", 0, &angle) &&
        render_number(L, fields, "size", 1, &scale) && scale > 0) {
        if (image) {
            MvImageRegion region = {0, 0, mv_image_width(image->image), mv_image_height(image->image)};
            MvImageTransform transform = {(float)x, (float)y, (float)image->pivot_x,
                                          (float)image->pivot_y, (float)scale, (float)angle};
            (void)mv_image_draw_region(image->image, region, transform);
        } else if (graphic->sprite) {
            int graph_index = lua_gettop(L);
            lua_getiuservalue(L, graph_index, 2);
            LuaImage *sprite_image = luaL_testudata(L, -1, "malvado.image");
            if (sprite_image) {
                MvImageTransform transform = {(float)x, (float)y, (float)graphic->pivot_x,
                                               (float)graphic->pivot_y, (float)scale, (float)angle};
                (void)mv_image_draw_region(sprite_image->image, graphic->region, transform);
            }
            lua_pop(L, 1);
        } else {
            (void)mv_window_draw_graphic(graphic->shape,
                (MvCollisionTransform){x, y, angle, scale}, graphic->color);
        }
    }
    lua_pop(L, 3);
}

static LuaCamera *
check_camera(lua_State *L, int index)
{
    LuaCamera *camera = luaL_checkudata(L, index, "malvado.camera");
    if (camera->closed)
        luaL_error(L, "camera is destroyed");
    return camera;
}

static void
camera_stop_following(lua_State *L, LuaCamera *camera)
{
    if (camera->target_ref >= 0)
        luaL_unref(L, LUA_REGISTRYINDEX, camera->target_ref);
    camera->target_ref = LUA_NOREF;
}

static int
camera_destroy(lua_State *L)
{
    LuaCamera *camera = luaL_checkudata(L, 1, "malvado.camera");
    camera_stop_following(L, camera);
    camera->closed = 1;
    return 0;
}

static int
camera_create(lua_State *L)
{
    double x = luaL_checknumber(L, 1), y = luaL_checknumber(L, 2);
    MvCameraViewport viewport, *view = NULL;
    if (!lua_isnoneornil(L, 3)) {
        validate_options(L, 3, "viewport", NULL);
        raw_option(L, 3, "viewport");
        if (!lua_isnil(L, -1)) {
            LuaRegion *region = luaL_checkudata(L, -1, "malvado.region");
            viewport = (MvCameraViewport){region->x, region->y, region->width, region->height};
            view = &viewport;
        }
        lua_pop(L, 1);
    }
    MvCamera value;
    if (!mv_camera_init(&value, x, y, view))
        return luaL_error(L, "invalid camera position or viewport");
    LuaCamera *camera = lua_newuserdatauv(L, sizeof(*camera), 0);
    *camera = (LuaCamera){.value = value, .target_ref = LUA_NOREF};
    luaL_setmetatable(L, "malvado.camera");
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.cameras");
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, (lua_Integer)lua_rawlen(L, -2) + 1);
    lua_pop(L, 1);
    return 1;
}

static int
camera_set(lua_State *L)
{
    int explicit = lua_gettop(L) == 3;
    if (!explicit && lua_gettop(L) != 2)
        return luaL_error(L, "expected camera_set(x, y) or camera_set(camera, x, y)");
    LuaCamera *camera = explicit ? check_camera(L, 1) : &engine.main_camera;
    double x = luaL_checknumber(L, explicit ? 2 : 1);
    double y = luaL_checknumber(L, explicit ? 3 : 2);
    if (!mv_camera_set(&camera->value, x, y))
        return luaL_error(L, "invalid camera position");
    camera_stop_following(L, camera);
    return 0;
}

static int
camera_move(lua_State *L)
{
    int explicit = lua_gettop(L) == 3;
    if (!explicit && lua_gettop(L) != 2)
        return luaL_error(L, "expected camera_move(dx, dy) or camera_move(camera, dx, dy)");
    LuaCamera *camera = explicit ? check_camera(L, 1) : &engine.main_camera;
    double x = luaL_checknumber(L, explicit ? 2 : 1);
    double y = luaL_checknumber(L, explicit ? 3 : 2);
    if (!mv_camera_move(&camera->value, x, y))
        return luaL_error(L, "invalid camera movement");
    camera_stop_following(L, camera);
    return 0;
}

static int
camera_position(lua_State *L)
{
    LuaCamera *camera = lua_isnoneornil(L, 1) ? &engine.main_camera : check_camera(L, 1);
    lua_pushnumber(L, camera->value.x);
    lua_pushnumber(L, camera->value.y);
    return 2;
}

static int
camera_follow(lua_State *L)
{
    int arguments = lua_gettop(L);
    if (arguments != 1 && arguments != 2)
        return luaL_error(L, "expected camera_follow(target) or camera_follow(camera, target)");
    LuaCamera *camera = arguments == 2 ? check_camera(L, 1) : &engine.main_camera;
    int target = arguments;
    lua_Integer id = check_process_id(L, target);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK)
        return luaL_error(L, "camera target is no longer alive");
    lua_pushvalue(L, target);
    int reference = luaL_ref(L, LUA_REGISTRYINDEX);
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK) {
        luaL_unref(L, LUA_REGISTRYINDEX, reference);
        return luaL_error(L, "camera target is no longer alive");
    }
    camera_stop_following(L, camera);
    camera->target_ref = reference;
    return 0;
}

static void
update_camera(lua_State *L, LuaCamera *camera)
{
    if (camera->closed || camera->target_ref < 0) return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, camera->target_ref);
    LuaSelf *target = luaL_testudata(L, -1, "malvado.process");
    MvProcessStatus status;
    if (!target || mv_process_status(engine.context, (MvProcessId)target->id, &status) != MV_OK) {
        lua_pop(L, 1);
        camera_stop_following(L, camera);
        return;
    }
    lua_getiuservalue(L, -1, 1);
    if (lua_istable(L, -1)) {
        refresh_process_transform(L, target, -1);
        raw_option(L, -1, "x");
        raw_option(L, -2, "y");
        int valid = (lua_isnil(L, -2) || lua_type(L, -2) == LUA_TNUMBER) &&
                    (lua_isnil(L, -1) || lua_type(L, -1) == LUA_TNUMBER);
        double x = lua_isnil(L, -2) ? 0 : lua_tonumber(L, -2);
        double y = lua_isnil(L, -1) ? 0 : lua_tonumber(L, -1);
        if (valid)
            (void)mv_camera_center(&camera->value, x, y, engine.width, engine.height);
        lua_pop(L, 2);
    }
    lua_pop(L, 2);
}

static void
update_cameras(lua_State *L)
{
    if (!lua_checkstack(L, 10)) {
        engine.failed = 1;
        return;
    }
    update_camera(L, &engine.main_camera);
    lua_getfield(L, LUA_REGISTRYINDEX, "malvado.cameras");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        LuaCamera *camera = luaL_testudata(L, -1, "malvado.camera");
        if (camera) update_camera(L, camera);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

/* Return NULL only for a closed/invalid explicit camera. Screen-space content
 * uses an identity camera so region and local scroll retain their contracts. */
static const MvCamera *
process_camera(lua_State *L, const Process *process)
{
    static const MvCamera screen_space = {0};
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
    lua_getiuservalue(L, -1, 1);
    raw_option(L, -1, "camera");
    LuaCamera *stored = luaL_testudata(L, -1, "malvado.camera");
    const MvCamera *camera;
    if (lua_isnil(L, -1)) camera = &engine.main_camera.value;
    else if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) camera = &screen_space;
    else camera = stored && !stored->closed ? &stored->value : NULL;
    lua_pop(L, 3);
    return camera;
}

/* Return 1 for no region, 2 for an active clip, or 0 when fully off-screen. */
static int
begin_process_region(lua_State *L, const Process *process, const MvCamera *camera)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
    lua_getiuservalue(L, -1, 1);
    lua_getfield(L, -1, "region");
    LuaRegion *stored = luaL_testudata(L, -1, "malvado.region");
    LuaRegion region = stored ? *stored : (LuaRegion){0};
    int present = !lua_isnil(L, -1);
    int valid = stored != NULL;
    lua_pop(L, 3);
    if (present && !valid) return 0;
    int width, height;
    if (!mv_window_size(&width, &height)) return 0;
    if (!present && !camera->viewport.width) return 1;
    MvCameraViewport process_region = {region.x, region.y, region.width, region.height};
    MvCameraViewport clipped;
    if (mv_camera_clip(camera, present ? &process_region : NULL, width, height, &clipped) <= 0)
        return 0;
    return mv_window_clip_begin(clipped.x, clipped.y, clipped.width, clipped.height) ? 2 : 0;
}

/* Return 1 without a scroll, 2 when active, or 0 for invalid state. */
static int
begin_process_scroll(lua_State *L, const Process *process, const MvCamera *camera)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
    lua_getiuservalue(L, -1, 1);
    lua_getfield(L, -1, "scroll");
    LuaScroll *stored = luaL_testudata(L, -1, "malvado.scroll");
    LuaScroll scroll = stored ? *stored : (LuaScroll){0};
    int present = !lua_isnil(L, -1);
    int valid = stored != NULL;
    lua_pop(L, 3);
    if (present && !valid) return 0;
    double x, y;
    if (!mv_camera_project(camera, 0, 0, scroll.x, scroll.y, &x, &y)) return 0;
    if (x == 0 && y == 0) return 1;
    return mv_window_scroll_begin((float)-x, (float)-y) ? 2 : 0;
}

/* Spawn appends increasing native IDs; tick cleanup preserves survivor order.
 * Keep ordered rendering O(n log n) without a second slot map or allocation. */
static int
compare_render_id(const void *key, const void *entry)
{
    MvProcessId id = *(const MvProcessId *)key;
    MvProcessId candidate = (MvProcessId)((const Process *)entry)->id;
    return (id > candidate) - (id < candidate);
}

static void
render(void)
{
    lua_State *L = engine.L;
    if (!lua_checkstack(L, 8)) {
        engine.failed = 1;
        return;
    }
    size_t count;
    if (mv_process_draw_order(engine.context, NULL, 0, &count) != MV_OK ||
        count > SIZE_MAX / sizeof(MvProcessDrawInfo)) {
        engine.failed = 1;
        return;
    }
    MvProcessDrawInfo *order = count ? malloc(count * sizeof(*order)) : NULL;
    if ((count && !order) ||
        mv_process_draw_order(engine.context, order, count, &count) != MV_OK) {
        free(order);
        engine.failed = 1;
        return;
    }
    mv_window_begin(engine.draw.background);
    for (size_t p = 0; p < count; p++) {
        Process *process = engine.count ? bsearch(&order[p].id, engine.items,
            engine.count, sizeof(*engine.items), compare_render_id) : NULL;
        if (!process) continue;
        if (!process->alive || process->state == PROCESS_SLEEPING || process->state == PROCESS_KILLED)
            continue;
        const MvCamera *camera = process_camera(L, process);
        if (!camera) continue;
        int region = begin_process_region(L, process, camera);
        if (!region) continue;
        int scroll = begin_process_scroll(L, process, camera);
        if (!scroll) {
            if (region == 2) (void)mv_window_clip_end();
            continue;
        }
        render_graph(L, process);
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
        if (scroll == 2) (void)mv_window_scroll_end();
        if (region == 2) (void)mv_window_clip_end();
    }
    render_texts(L);
    (void)mv_window_draw_fade(engine.fade.fade.current);
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
        if (!mv_window_open(w, h, title)) {
            engine.failed = 1;
            return luaL_error(L, "could not open window");
        }
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
    "camera_create", "camera_set", "camera_move", "camera_follow",
    "collision", "world_create", "world_move",
    "rect", "circle", "region", "scroll",
    "rect_overlaps",
    "rect_contains",
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
    "key_pressed",
    "key_released",
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
    "animation",
    "graph",
    "load_asset",
    "load_image",
    "image_size",
    "draw_image",
    "draw_image_region",
    "load_atlas",
    "load_aseprite",
    "sprite_info",
    "sprite_id",
    "animation_frame",
    "animation_color",
    "atlas_pivot_color",
    "animation_repeats",
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

static MvRect
check_rect(lua_State *L, int index)
{
    MvRect rect = {luaL_checknumber(L, index), luaL_checknumber(L, index + 1),
                   luaL_checknumber(L, index + 2), luaL_checknumber(L, index + 3)};
    return rect;
}

static int
rect_overlaps(lua_State *L)
{
    MvRect a = check_rect(L, 1);
    MvRect b = check_rect(L, 5);
    int result = mv_rect_overlaps(a, b);
    if (result < 0)
        return luaL_error(L, "invalid rectangle");
    lua_pushboolean(L, result);
    return 1;
}

static int
rect_contains(lua_State *L)
{
    MvRect rect = check_rect(L, 1);
    double x = luaL_checknumber(L, 5);
    double y = luaL_checknumber(L, 6);
    int result = mv_rect_contains(rect, x, y);
    if (result < 0)
        return luaL_error(L, "invalid rectangle or point");
    lua_pushboolean(L, result);
    return 1;
}

static double
shape_option(lua_State *L, int index, const char *name, double fallback)
{
    raw_option(L, index, name);
    double value = lua_isnil(L, -1) ? fallback : luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return value;
}

static int
collision_shape(lua_State *L)
{
    MvCollisionKind kind = (MvCollisionKind)lua_tointeger(L, lua_upvalueindex(1));
    MvCollisionShape shape = {.kind = kind};
    int options;
    if (kind == MV_COLLISION_CIRCLE) {
        shape.radius = luaL_checknumber(L, 1);
        options = 2;
    } else {
        shape.width = luaL_checknumber(L, 1);
        shape.height = luaL_checknumber(L, 2);
        options = 3;
    }
    if (!lua_isnoneornil(L, options)) {
        validate_options(L, options, "offset_x", "offset_y");
        shape.offset_x = shape_option(L, options, "offset_x", 0);
        shape.offset_y = shape_option(L, options, "offset_y", 0);
    }
    MvCollisionTransform identity = {0, 0, 0, 1};
    if (mv_collision_overlap(shape, identity, shape, identity) < 0)
        return luaL_error(L, "invalid collision shape");
    MvCollisionShape *stored = lua_newuserdatauv(L, sizeof(*stored), 0);
    *stored = shape;
    luaL_setmetatable(L, "malvado.collision_shape");
    return 1;
}

static MvCollisionTransform
collision_transform(lua_State *L, int index)
{
    MvCollisionTransform result = {0, 0, 0, 1};
    if (lua_isnoneornil(L, index))
        return result;
    luaL_checktype(L, index, LUA_TTABLE);
    index = lua_absindex(L, index);
    lua_pushnil(L);
    while (lua_next(L, index)) {
        size_t length;
        const char *name = lua_type(L, -2) == LUA_TSTRING ? lua_tolstring(L, -2, &length) : NULL;
        if (!name || strlen(name) != length ||
            (strcmp(name, "x") && strcmp(name, "y") && strcmp(name, "angle") && strcmp(name, "scale")))
            luaL_error(L, "unknown collision transform field");
        lua_pop(L, 1);
    }
    result.x = shape_option(L, index, "x", 0);
    result.y = shape_option(L, index, "y", 0);
    result.angle = shape_option(L, index, "angle", 0);
    result.scale = shape_option(L, index, "scale", 1);
    return result;
}

static int
collision_overlap(lua_State *L)
{
    MvCollisionShape *a = luaL_checkudata(L, 1, "malvado.collision_shape");
    MvCollisionTransform transform_a = collision_transform(L, 2);
    MvCollisionShape *b = luaL_checkudata(L, 3, "malvado.collision_shape");
    MvCollisionTransform transform_b = collision_transform(L, 4);
    int result = mv_collision_overlap(*a, transform_a, *b, transform_b);
    if (result < 0)
        return luaL_error(L, "invalid collision transform");
    lua_pushboolean(L, result);
    return 1;
}

static int
world_gc(lua_State *L)
{
    MvCollisionWorld **world = luaL_checkudata(L, 1, "malvado.collision_world");
    mv_collision_world_destroy(*world);
    *world = NULL;
    return 0;
}

static MvCollisionWorld *
check_world(lua_State *L)
{
    MvCollisionWorld **world = luaL_checkudata(L, 1, "malvado.collision_world");
    if (!*world)
        luaL_error(L, "collision world is closed");
    return *world;
}

static int
world_create(lua_State *L)
{
    MvCollisionWorld **world = lua_newuserdatauv(L, sizeof(*world), 0);
    *world = NULL;
    luaL_setmetatable(L, "malvado.collision_world");
    *world = mv_collision_world_create();
    if (!*world)
        return luaL_error(L, "cannot allocate collision world");
    return 1;
}

static double
world_rect_field(lua_State *L, const char *name)
{
    raw_option(L, 2, name);
    double value = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return value;
}

static int
world_add_rect(lua_State *L)
{
    MvCollisionWorld *world = check_world(L);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, 2)) {
        size_t length;
        const char *name = lua_type(L, -2) == LUA_TSTRING ? lua_tolstring(L, -2, &length) : NULL;
        if (!name || strlen(name) != length ||
            (strcmp(name, "x") && strcmp(name, "y") && strcmp(name, "width") && strcmp(name, "height")))
            return luaL_error(L, "unknown world rectangle field");
        lua_pop(L, 1);
    }
    MvRect rect = {world_rect_field(L, "x"), world_rect_field(L, "y"),
                   world_rect_field(L, "width"), world_rect_field(L, "height")};
    size_t id;
    if (!mv_collision_world_add_rect(world, rect, &id))
        return luaL_error(L, "invalid rectangle or out of memory");
    lua_pushinteger(L, (lua_Integer)id);
    return 1;
}

static int
world_overlap(lua_State *L)
{
    MvCollisionWorld *world = check_world(L);
    MvCollisionShape *shape = luaL_checkudata(L, 2, "malvado.collision_shape");
    MvCollisionTransform transform = collision_transform(L, 3);
    size_t id;
    int result = mv_collision_world_query(world, *shape, transform, &id);
    if (result < 0)
        return luaL_error(L, "invalid collision transform");
    if (!result)
        lua_pushnil(L);
    else
        lua_pushinteger(L, (lua_Integer)id);
    return 1;
}

static int
process_collider(lua_State *L, int fields, int legacy, MvCollisionShape *shape)
{
    raw_option(L, fields, "collider");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        if (legacy) {
            raw_option(L, fields, "fpg");
            int has_sheet = !lua_isnil(L, -1);
            lua_pop(L, 1);
            if (has_sheet)
                return luaL_error(L, "legacy sheets require an explicit collider");
        }
        raw_option(L, fields, "graph");
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            return 0;
        }
        LuaGraphic *graphic = luaL_testudata(L, -1, "malvado.graphic");
        LuaImage *image = luaL_testudata(L, -1, "malvado.image");
        LuaAnimation *animation = luaL_testudata(L, -1, "malvado.animation");
        if (animation) {
            *shape = animation->shape;
        } else if (graphic) {
            *shape = graphic->shape;
        } else if (image && image->image) {
            int width = mv_image_width(image->image), height = mv_image_height(image->image);
            if (width <= 0 || height <= 0)
                return luaL_error(L, "invalid image for automatic collider");
            *shape = (MvCollisionShape){.kind = MV_COLLISION_RECT,
                .width = width, .height = height, .offset_x = legacy ? 0 : width / 2.0 - image->pivot_x,
                .offset_y = legacy ? 0 : height / 2.0 - image->pivot_y};
        } else {
            return luaL_error(L, "unsupported graphic for automatic collider");
        }
    } else if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
        lua_pop(L, 1);
        return 0;
    } else {
        MvCollisionShape *explicit_shape = luaL_checkudata(L, -1, "malvado.collision_shape");
        *shape = *explicit_shape;
    }
    lua_pop(L, 1);
    return 1;
}

static int
world_process_collision(lua_State *L)
{
    lua_settop(L, 3);
    MvCollisionWorld *world = check_world(L);
    if (!lua_isnil(L, 3)) {
        validate_options(L, 3, "x", "y");
        double x = shape_option(L, 3, "x", 0);
        double y = shape_option(L, 3, "y", 0);
        if (!isfinite(x) || !isfinite(y))
            return luaL_error(L, "invalid collision position");
    }
    lua_Integer id = check_process_id(L, 2);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    lua_getiuservalue(L, 2, 1);
    int fields = lua_gettop(L);
    refresh_process_transform(L, luaL_checkudata(L, 2, "malvado.process"), fields);
    MvCollisionShape shape;
    Process *slot = find_process_slot(id);
    if (!process_collider(L, fields, slot && slot->legacy_frame, &shape)) {
        lua_pushnil(L);
        return 1;
    }
    MvCollisionTransform transform = {
        shape_option(L, fields, "x", 0), shape_option(L, fields, "y", 0),
        shape_option(L, fields, "angle", 0), shape_option(L, fields, "size", 1)
    };
    if (!lua_isnil(L, 3)) {
        transform.x = shape_option(L, 3, "x", transform.x);
        transform.y = shape_option(L, 3, "y", transform.y);
    }
    size_t hit;
    int result = mv_collision_world_query(world, shape, transform, &hit);
    if (result < 0)
        return luaL_error(L, "invalid process collision transform");
    if (result)
        lua_pushinteger(L, (lua_Integer)hit);
    else
        lua_pushnil(L);
    return 1;
}

/* Context belongs to the scheduled coroutine. A nested user coroutine must
 * not borrow the active process of its caller. */
static void
push_current_process(lua_State *L)
{
    if (!engine.active || L != engine.active_thread)
        luaL_error(L, "operation requires a running process");
    Process *process = find_process(engine.active_id);
    if (!process)
        luaL_error(L, "current process is no longer alive");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->self_ref);
}

static int
world_process_move(lua_State *L)
{
    lua_settop(L, 4);
    MvCollisionWorld *world = check_world(L);
    lua_Integer id = check_process_id(L, 2);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    double dx = luaL_checknumber(L, 3), dy = luaL_checknumber(L, 4);
    if (!isfinite(dx) || !isfinite(dy) || fabs(dx) > DBL_MAX / 16 || fabs(dy) > DBL_MAX / 16)
        return luaL_error(L, "invalid movement");
    lua_getiuservalue(L, 2, 1);
    int fields = lua_gettop(L);
    refresh_process_transform(L, luaL_checkudata(L, 2, "malvado.process"), fields);
    MvCollisionTransform transform = {
        shape_option(L, fields, "x", 0), shape_option(L, fields, "y", 0),
        shape_option(L, fields, "angle", 0), shape_option(L, fields, "size", 1)
    };
    if (!isfinite(transform.x) || !isfinite(transform.y))
        return luaL_error(L, "invalid process position");
    double old_x = transform.x, old_y = transform.y;
    MvCollisionShape shape;
    MvCollisionSweep result = {.fraction = 1};
    Process *slot = find_process_slot(id);
    if (!process_collider(L, fields, slot && slot->legacy_frame, &shape)) {
        transform.x += dx;
        transform.y += dy;
    } else if (mv_collision_world_move(world, shape, &transform, dx, dy, &result) < 0) {
        return luaL_error(L, "invalid movement or initial collision");
    }
    if (!isfinite(transform.x) || !isfinite(transform.y))
        return luaL_error(L, "invalid movement destination");
    lua_createtable(L, 0, 6);
    lua_pushboolean(L, result.hit);
    lua_setfield(L, -2, "hit");
    lua_pushnumber(L, transform.x - old_x);
    lua_setfield(L, -2, "moved_x");
    lua_pushnumber(L, transform.y - old_y);
    lua_setfield(L, -2, "moved_y");
    lua_pushnumber(L, result.normal_x);
    lua_setfield(L, -2, "normal_x");
    lua_pushnumber(L, result.normal_y);
    lua_setfield(L, -2, "normal_y");
    if (result.hit) {
        lua_pushinteger(L, (lua_Integer)result.id);
        lua_setfield(L, -2, "id");
    }
    LuaSelf *self = luaL_checkudata(L, 2, "malvado.process");
    if (mv_process_set_position(engine.context, self->reference, transform.x, transform.y) != MV_OK)
        return luaL_error(L, "cannot set process position");
    lua_pushliteral(L, "x");
    lua_pushnumber(L, transform.x);
    lua_rawset(L, fields);
    lua_pushliteral(L, "y");
    lua_pushnumber(L, transform.y);
    lua_rawset(L, fields);
    sync_process_collider(L, self, fields);
    return 1;
}

static int
world_move(lua_State *L)
{
    int arguments = lua_gettop(L);
    if (arguments == 2) {
        push_current_process(L);
        lua_insert(L, 1);
        lua_getiuservalue(L, 1, 1);
        raw_option(L, -1, "world");
        lua_remove(L, -2);
        if (lua_isnil(L, -1))
            return luaL_error(L, "current process has no world");
        if (!luaL_testudata(L, -1, "malvado.collision_world"))
            return luaL_error(L, "current process world must be a collision world");
        lua_insert(L, 1);
    } else if (arguments != 4) {
        return luaL_error(L, "expected world_move(dx, dy) or world_move(world, process, dx, dy)");
    }
    return world_process_move(L);
}

static int
world_sweep(lua_State *L)
{
    MvCollisionWorld *world = check_world(L);
    MvCollisionShape *shape = luaL_checkudata(L, 2, "malvado.collision_shape");
    MvCollisionTransform transform = collision_transform(L, 3);
    double dx = luaL_checknumber(L, 4);
    double dy = luaL_checknumber(L, 5);
    MvCollisionSweep result;
    if (mv_collision_world_sweep(world, *shape, transform, dx, dy, &result) < 0)
        return luaL_error(L, "invalid sweep, unsupported shape, or initial overlap");
    lua_createtable(L, 0, 5);
    lua_pushboolean(L, result.hit);
    lua_setfield(L, -2, "hit");
    if (result.hit) {
        lua_pushinteger(L, (lua_Integer)result.id);
        lua_setfield(L, -2, "id");
    }
    lua_pushnumber(L, result.fraction);
    lua_setfield(L, -2, "fraction");
    lua_pushnumber(L, result.normal_x);
    lua_setfield(L, -2, "normal_x");
    lua_pushnumber(L, result.normal_y);
    lua_setfield(L, -2, "normal_y");
    return 1;
}

static int
process_collision_query(lua_State *L, int multiple)
{
    LuaSelf *source = luaL_checkudata(L, 1, "malvado.process");
    lua_Integer source_id = check_process_id(L, 1);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)source_id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    /* Native hosts can move Lua processes between ticks. Refresh before any
     * query, including sleeping/frozen targets and constructor filters. */
    for (size_t i = 0; i < engine.count; ++i) {
        if (!engine.items[i].alive) continue;
        lua_rawgeti(L, LUA_REGISTRYINDEX, engine.items[i].self_ref);
        LuaSelf *self = lua_touserdata(L, -1);
        lua_getiuservalue(L, -1, 1);
        sync_process_collider(L, self, -1);
        lua_pop(L, 2);
    }
    if (source->collision_invalid)
        return luaL_error(L, "invalid process collider or transform");
    MvProcessRef source_ref = source->reference;
    uint64_t type = 0;
    LuaSelf *target = luaL_testudata(L, 2, "malvado.process");
    if (target) {
        (void)check_process_id(L, 2);
        if (multiple)
            lua_createtable(L, 1, 0);
        if (mv_process_status(engine.context, (MvProcessId)source_id, &status) != MV_OK)
            return luaL_error(L, "process is no longer alive");
        if (source->collision_invalid)
            return luaL_error(L, "invalid process collider or transform");
        MvProcessStatus target_status;
        if (target->collision_invalid &&
            mv_process_status(engine.context, (MvProcessId)target->id, &target_status) == MV_OK)
            return luaL_error(L, "invalid process collider or transform");
        int hit = 0;
        MvResult result = mv_process_collider_test(engine.context, source_ref, target->reference, &hit);
        if (result != MV_OK && result != MV_NOT_FOUND)
            return luaL_error(L, "invalid collision target");
        if (multiple) {
            if (hit) {
                lua_pushvalue(L, 2);
                lua_seti(L, -2, 1);
            }
        } else if (hit) {
            lua_pushvalue(L, 2);
        } else lua_pushnil(L);
        return 1;
    }
    if (!lua_iscfunction(L, 2) || lua_tocfunction(L, 2) != create_process ||
        !lua_getupvalue(L, 2, 2))
        return luaL_error(L, "expected a process constructor or handle");
    type = (uint64_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (!multiple) {
        MvProcessRef found;
        if (mv_process_collider_first(engine.context, source_ref, type, &found) != MV_OK)
            return luaL_error(L, "failed to query process collisions");
        if (!found.id) {
            lua_pushnil(L);
        } else {
            MvProcessInfo info;
            if (mv_process_info(engine.context, found, &info) != MV_OK)
                return luaL_error(L, "collision target is no longer alive");
            push_process_handle(L, &info);
            if (mv_process_status(engine.context, (MvProcessId)source_id, &status) != MV_OK)
                return luaL_error(L, "process is no longer alive");
            if (mv_process_status(engine.context, found.id, &status) != MV_OK) {
                lua_pop(L, 1);
                lua_pushnil(L);
            }
        }
        return 1;
    }
    size_t capacity = mv_process_count(engine.context), total = 0;
    MvProcessRef *refs = NULL;
    int captured = 0;
    for (int attempt = 0; attempt < 16 && !captured; ++attempt) {
        if (capacity > SIZE_MAX / sizeof(*refs))
            return luaL_error(L, "too many processes");
        refs = lua_newuserdatauv(L, capacity * sizeof(*refs), 0);
        if (mv_process_collider_find(engine.context, source_ref, type, refs, capacity, &total) != MV_OK)
            return luaL_error(L, "failed to query process collisions");
        if (total > capacity) {
            lua_pop(L, 1);
            capacity = total;
        } else captured = 1;
    }
    if (!captured)
        return luaL_error(L, "process list changed during collision query");
    lua_newtable(L);
    lua_Integer count = 0;
    for (size_t i = 0; i < total; ++i) {
        MvProcessInfo info;
        if (mv_process_info(engine.context, refs[i], &info) == MV_OK) {
            push_process_handle(L, &info);
            lua_seti(L, -2, ++count);
            if (mv_process_status(engine.context, refs[i].id, &status) != MV_OK) {
                lua_pushnil(L);
                lua_seti(L, -2, count--);
            }
        }
    }
    if (mv_process_status(engine.context, (MvProcessId)source_id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    return 1;
}

static int
process_collision_first(lua_State *L)
{
    return process_collision_query(L, 0);
}

static int
process_collision_all(lua_State *L)
{
    return process_collision_query(L, 1);
}

static int
call_collision_namespace(lua_State *L)
{
    /* __call supplies the namespace before the single query target. */
    if (lua_gettop(L) != 2)
        return luaL_error(L, "expected collision(target)");
    lua_remove(L, 1);
    push_current_process(L);
    lua_insert(L, 1);
    return process_collision_first(L);
}

static int
graphic_create(lua_State *L)
{
    MvCollisionKind kind = (MvCollisionKind)lua_tointeger(L, lua_upvalueindex(1));
    LuaGraphic graphic = {.shape = {.kind = kind}, .color = {255, 255, 255, 255}};
    int options;
    if (kind == MV_COLLISION_CIRCLE) {
        graphic.shape.radius = luaL_checknumber(L, 1);
        options = 2;
    } else {
        graphic.shape.width = luaL_checknumber(L, 1);
        graphic.shape.height = luaL_checknumber(L, 2);
        options = 3;
    }
    int valid = kind == MV_COLLISION_CIRCLE
        ? mv_graphic_circle(graphic.shape.radius, &graphic.shape)
        : mv_graphic_rect(graphic.shape.width, graphic.shape.height, 0, &graphic.shape);
    if (!valid)
        return luaL_error(L, "invalid graphic dimensions");
    if (!lua_isnoneornil(L, options)) {
        validate_options(L, options, "color", "pivot");
        raw_option(L, options, "pivot");
        if (!lua_isnil(L, -1)) {
            size_t length;
            const char *pivot = luaL_checklstring(L, -1, &length);
            if (strlen(pivot) != length ||
                (strcmp(pivot, "center") && (kind != MV_COLLISION_RECT || strcmp(pivot, "top-left"))))
                return luaL_error(L, "invalid graphic pivot");
            graphic.top_left = strcmp(pivot, "top-left") == 0;
        }
        lua_pop(L, 1);
        raw_option(L, options, "color");
        if (!lua_isnil(L, -1)) {
            luaL_checktype(L, -1, LUA_TTABLE);
            int color = lua_absindex(L, -1);
            lua_pushnil(L);
            while (lua_next(L, color)) {
                if (!lua_isinteger(L, -2) || lua_tointeger(L, -2) < 1 || lua_tointeger(L, -2) > 4)
                    return luaL_error(L, "invalid graphic color");
                lua_pop(L, 1);
            }
            unsigned char channels[4] = {255, 255, 255, 255};
            for (int i = 0; i < 4; ++i) {
                lua_rawgeti(L, color, i + 1);
                if (i < 3 || !lua_isnil(L, -1)) {
                    if (!lua_isinteger(L, -1) || lua_tointeger(L, -1) < 0 || lua_tointeger(L, -1) > 255)
                        return luaL_error(L, "invalid graphic color");
                    channels[i] = (unsigned char)lua_tointeger(L, -1);
                }
                lua_pop(L, 1);
            }
            graphic.color = (MvColor){channels[0], channels[1], channels[2], channels[3]};
        }
        lua_pop(L, 1);
    }
    if (graphic.top_left) {
        (void)mv_graphic_rect(graphic.shape.width, graphic.shape.height, 1, &graphic.shape);
    }
    LuaGraphic *stored = lua_newuserdatauv(L, sizeof(*stored), 0);
    *stored = graphic;
    luaL_setmetatable(L, "malvado.graphic");
    return 1;
}

static int
scroll_create(lua_State *L)
{
    lua_Number x = luaL_checknumber(L, 1), y = luaL_checknumber(L, 2);
    if (!isfinite(x) || !isfinite(y) || fabs(x) > FLT_MAX / 16 || fabs(y) > FLT_MAX / 16)
        return luaL_error(L, "invalid scroll coordinates");
    LuaScroll *scroll = lua_newuserdatauv(L, sizeof(*scroll), 0);
    *scroll = (LuaScroll){(float)x, (float)y};
    luaL_setmetatable(L, "malvado.scroll");
    return 1;
}

static int
region_create(lua_State *L)
{
    lua_Integer x = luaL_checkinteger(L, 1), y = luaL_checkinteger(L, 2);
    lua_Integer width = luaL_checkinteger(L, 3), height = luaL_checkinteger(L, 4);
    if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX ||
        width <= 0 || width > INT_MAX || height <= 0 || height > INT_MAX)
        return luaL_error(L, "invalid region rectangle");
    LuaRegion *region = lua_newuserdatauv(L, sizeof(*region), 0);
    *region = (LuaRegion){(int)x, (int)y, (int)width, (int)height};
    luaL_setmetatable(L, "malvado.region");
    return 1;
}

#ifdef MALVADO_BUILD_ASSETS
static int
push_sprite_graphic(lua_State *L, MvImageRegion region, int pivot_x, int pivot_y, int image)
{
    image = lua_absindex(L, image);
    LuaGraphic *graphic = lua_newuserdatauv(L, sizeof(*graphic), 2);
    *graphic = (LuaGraphic){
        .shape = {.kind = MV_COLLISION_RECT, .width = region.width,
                  .height = region.height,
                  .offset_x = region.width / 2.0 - pivot_x,
                  .offset_y = region.height / 2.0 - pivot_y},
        .sprite = 1, .region = region, .pivot_x = pivot_x, .pivot_y = pivot_y
    };
    luaL_setmetatable(L, "malvado.graphic");
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    lua_pushvalue(L, image);
    lua_setiuservalue(L, -2, 2);
    return 1;
}

static int
graphic_sprite(lua_State *L)
{
    LuaAtlas *atlas = luaL_checkudata(L, 1, "malvado.atlas");
    lua_Integer id = luaL_checkinteger(L, 2);
    MvAtlasSprite sprite;
    if (id <= 0 || id > INT_MAX || !mv_atlas_find(atlas->atlas, (int)id, &sprite))
        return luaL_error(L, "unknown sprite id");
    lua_getiuservalue(L, 1, 1);
    return push_sprite_graphic(L, sprite.region, sprite.pivot_x, sprite.pivot_y, -1);
}

static int
graph_select(lua_State *L)
{
    int arguments = lua_gettop(L);
    if (arguments == 1) {
        push_current_process(L);
        lua_getiuservalue(L, -1, 1);
        raw_option(L, -1, "file");
        if (lua_isnil(L, -1))
            return luaL_error(L, "current process has no file");
        lua_replace(L, 2);
        lua_settop(L, 2);
        lua_insert(L, 1);
    } else if (arguments != 2) {
        return luaL_error(L, "expected graph(id) or graph(library, id)");
    }
    lua_Integer id = luaL_checkinteger(L, 2);
    if (id <= 0 || id > INT_MAX)
        return luaL_error(L, "unknown graph id");
    LuaAtlas *atlas = luaL_testudata(L, 1, "malvado.atlas");
    if (atlas) {
        if (engine.window) {
            lua_getiuservalue(L, 1, 1);
            LuaImage *image = luaL_testudata(L, -1, "malvado.image");
            if (!image || !image->image)
                return luaL_error(L, "asset has no visual images; load_asset requires an open window");
            lua_pop(L, 1);
        }
        return graphic_sprite(L);
    }
    LuaFpg *fpg = luaL_testudata(L, 1, "malvado.fpg");
    LuaFpgManifest *manifest = luaL_testudata(L, 1, "malvado.fpg_manifest");
    if (!fpg && !manifest)
        return luaL_error(L, "graph requires an atlas, FPG, or loaded FPG manifest");
    MvImageRegion region = {0};
    MvFpgPoint pivot = {0};
    if (fpg) {
        MvFpgSprite sprite;
        if (!mv_fpg_sprite(fpg->fpg, (int)id, &sprite))
            return luaL_error(L, "unknown graph id");
        region.width = sprite.width;
        region.height = sprite.height;
        (void)mv_fpg_point(fpg->fpg, (int)id, 0, &pivot);
    } else {
        MvFpgManifestSprite sprite;
        if (!mv_fpg_manifest_sprite(manifest->manifest, (int)id, &sprite))
            return luaL_error(L, "unknown graph id");
        region.width = sprite.width;
        region.height = sprite.height;
        (void)mv_fpg_manifest_point(manifest->manifest, (int)id, 0, &pivot);
    }
    if (!pivot.defined)
        pivot.x = pivot.y = 0;
    int cache_slot = fpg ? 1 : 2;
    lua_getiuservalue(L, 1, cache_slot);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setiuservalue(L, 1, cache_slot);
    }
    int cache = lua_gettop(L);
    lua_rawgeti(L, cache, id);
    if (lua_isnil(L, -1) && engine.window) {
        if (!fpg)
            return luaL_error(L, "manifest has no visual images; use load_asset with an open window");
        lua_pop(L, 1);
        lua_pushcfunction(L, fpg_image);
        lua_pushvalue(L, 1);
        lua_pushinteger(L, id);
        lua_pushboolean(L, fpg->transparent_zero);
        lua_call(L, 3, 1);
        lua_pushvalue(L, -1);
        lua_rawseti(L, cache, id);
    }
    return push_sprite_graphic(L, region, pivot.x, pivot.y, -1);
}

static int
animation_gc(lua_State *L)
{
    LuaAnimation *animation = luaL_checkudata(L, 1, "malvado.animation");
    mv_animation_destroy(animation->sequence);
    animation->sequence = NULL;
    return 0;
}

static int
animation_sample(lua_State *L)
{
    LuaAnimation *animation = luaL_checkudata(L, 1, "malvado.animation");
    double elapsed = luaL_checknumber(L, 2);
    if (!isfinite(elapsed) || elapsed < 0 || elapsed > 1e12)
        return luaL_error(L, "invalid animation sample time in milliseconds");
    MvAnimationSample sample;
    if (!mv_animation_sample(animation->sequence, (uint64_t)(elapsed * 1000), &sample))
        return luaL_error(L, "invalid animation descriptor");
    lua_getiuservalue(L, 1, 1);
    lua_rawgeti(L, -1, (lua_Integer)sample.index + 1);
    return 1;
}

static int
animation_elapsed(lua_State *L)
{
    lua_Integer id = check_process_id(L, 1);
    MvProcessStatus status;
    if (mv_process_status(engine.context, (MvProcessId)id, &status) != MV_OK)
        return luaL_error(L, "process is no longer alive");
    LuaSelf *self = luaL_checkudata(L, 1, "malvado.process");
    lua_getiuservalue(L, 1, 1);
    raw_option(L, -1, "graph");
    if (luaL_testudata(L, -1, "malvado.animation"))
        lua_pushnumber(L, self->animation_state.elapsed_us / 1000.0);
    else lua_pushnil(L);
    return 1;
}

static int
animation_select(lua_State *L)
{
    int arguments = lua_gettop(L);
    if (lua_type(L, 1) == LUA_TSTRING || lua_istable(L, 1)) {
        if (arguments != 1 && arguments != 2)
            return luaL_error(L, "expected animation(sequence, options?)");
        push_current_process(L);
        lua_getiuservalue(L, -1, 1);
        raw_option(L, -1, "file");
        if (lua_isnil(L, -1)) return luaL_error(L, "current process has no file");
        lua_remove(L, -2);
        lua_remove(L, -2);
        lua_insert(L, 1);
    } else if (arguments != 2 && arguments != 3) {
        return luaL_error(L, "expected animation(library, sequence, options?)");
    }
    lua_settop(L, 3);
    int loop = 1, has_fps = 0;
    double fps = 8;
    if (!lua_isnil(L, 3)) {
        validate_options(L, 3, "fps", "loop");
        raw_option(L, 3, "loop");
        if (!lua_isnil(L, -1)) {
            if (!lua_isboolean(L, -1)) return luaL_error(L, "loop must be a boolean");
            loop = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        raw_option(L, 3, "fps");
        has_fps = !lua_isnil(L, -1);
        if (has_fps) fps = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    int named = lua_type(L, 2) == LUA_TSTRING;
    LuaAtlas *atlas = named ? luaL_checkudata(L, 1, "malvado.atlas") : NULL;
    size_t count;
    const char *name = NULL;
    int cache = 0;
    if (named) {
        if (engine.window) {
            lua_getiuservalue(L, 1, 1);
            LuaImage *image = luaL_testudata(L, -1, "malvado.image");
            if (!image || !image->image)
                return luaL_error(L, "asset has no visual images; load_asset requires an open window");
            lua_pop(L, 1);
        }
        if (has_fps) return luaL_error(L, "fps is only valid for custom frame lists");
        size_t length;
        name = luaL_checklstring(L, 2, &length);
        if (!length || strlen(name) != length) return luaL_error(L, "invalid animation name");
        count = mv_atlas_animation_frame_count(atlas->atlas, name);
        if (!count) return luaL_error(L, "unknown animation");
        lua_getiuservalue(L, 1, 2);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setiuservalue(L, 1, 2);
        }
        int variants = lua_gettop(L);
        lua_rawgeti(L, variants, loop + 1);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_newtable(L);
            lua_pushliteral(L, "v");
            lua_setfield(L, -2, "__mode");
            lua_setmetatable(L, -2);
            lua_pushvalue(L, -1);
            lua_rawseti(L, variants, loop + 1);
        }
        cache = lua_gettop(L);
        lua_pushvalue(L, 2);
        lua_rawget(L, cache);
        if (!lua_isnil(L, -1)) return 1;
        lua_pop(L, 1);
    } else {
        luaL_checktype(L, 2, LUA_TTABLE);
        count = lua_rawlen(L, 2);
        if (!count || count > 4096) return luaL_error(L, "animation needs 1..4096 frames");
        lua_pushnil(L);
        while (lua_next(L, 2)) {
            if (!lua_isinteger(L, -2) || lua_tointeger(L, -2) < 1 ||
                (uint64_t)lua_tointeger(L, -2) > count)
                return luaL_error(L, "animation frames must be a dense ID list");
            lua_pop(L, 1);
        }
        if (!isfinite(fps) || fps < 1.0 / 3600 || fps > 1000)
            return luaL_error(L, "animation fps must be in 1/3600..1000");
    }
    if (count > 8192) return luaL_error(L, "animation needs at most 8192 expanded frames");
    LuaAnimation *animation = lua_newuserdatauv(L, sizeof(*animation), 2);
    *animation = (LuaAnimation){0};
    luaL_setmetatable(L, "malvado.animation");
    int descriptor = lua_gettop(L);
    MvAnimationFrame *frames = lua_newuserdatauv(L, count * sizeof(*frames), 0);
    lua_createtable(L, (int)count, 0);
    int graphics = lua_gettop(L);
    double left = 0, top = 0, right = 0, bottom = 0;
    for (size_t i = 0; i < count; ++i) {
        int id;
        uint64_t duration;
        if (named) {
            MvAtlasAnimationFrame frame;
            if (!mv_atlas_animation_frame_at(atlas->atlas, name, i, &frame))
                return luaL_error(L, "invalid animation sequence");
            id = frame.sprite_id;
            duration = (uint64_t)frame.duration_ms * 1000;
        } else {
            lua_rawgeti(L, 2, (lua_Integer)i + 1);
            lua_Integer value = luaL_checkinteger(L, -1);
            if (value <= 0 || value > INT_MAX) return luaL_error(L, "invalid animation graph id");
            id = (int)value;
            lua_pop(L, 1);
            duration = (uint64_t)floor(1000000.0 / fps + 0.5);
        }
        frames[i] = (MvAnimationFrame){id, duration};
        lua_pushcfunction(L, graph_select);
        lua_pushvalue(L, 1);
        lua_pushinteger(L, id);
        lua_call(L, 2, 1);
        LuaGraphic *graphic = lua_touserdata(L, -1);
        double x = graphic->shape.offset_x - graphic->shape.width / 2;
        double y = graphic->shape.offset_y - graphic->shape.height / 2;
        double end_x = x + graphic->shape.width, end_y = y + graphic->shape.height;
        if (!i || x < left) left = x;
        if (!i || y < top) top = y;
        if (!i || end_x > right) right = end_x;
        if (!i || end_y > bottom) bottom = end_y;
        lua_rawseti(L, graphics, (lua_Integer)i + 1);
    }
    animation->sequence = mv_animation_create(frames, count, loop);
    if (!animation->sequence) return luaL_error(L, "cannot create animation definition");
    animation->shape = (MvCollisionShape){.kind = MV_COLLISION_RECT,
        .width = right - left, .height = bottom - top,
        .offset_x = (right + left) / 2, .offset_y = (bottom + top) / 2};
    lua_setiuservalue(L, descriptor, 1);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, descriptor, 2);
    if (cache) {
        lua_pushvalue(L, 2);
        lua_pushvalue(L, descriptor);
        lua_rawset(L, cache);
    }
    lua_pushvalue(L, descriptor);
    return 1;
}

#endif

static int
open_module(lua_State *L)
{
    static const luaL_Reg functions[] = {
        {"camera_follow", camera_follow},
        {"camera_create", camera_create},
        {"camera_set", camera_set},
        {"camera_move", camera_move},
        {"world_create", world_create},
        {"world_move", world_move},
        {"rect_overlaps", rect_overlaps},
        {"rect_contains", rect_contains},
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
        {"key_pressed", key_pressed},
        {"key_released", key_released},
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
        {"load_asset", load_asset},
        {"load_image", load_image},
        {"image_size", image_size},
        {"draw_image", draw_image},
        {"draw_image_region", draw_image_region},
        {"legacy_fpg", legacy_fpg},
#ifdef MALVADO_BUILD_ASSETS
        {"animation", animation_select},
        {"graph", graph_select},
        {"load_atlas", load_atlas},
        {"load_aseprite", load_aseprite},
        {"sprite_info", sprite_info},
        {"sprite_id", sprite_id},
        {"animation_frame", animation_frame},
        {"animation_color", animation_color},
        {"atlas_pivot_color", atlas_pivot_color},
        {"animation_repeats", animation_repeats},
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
    /* Keep collider field names interned: synchronization after native spawn
     * must not allocate or turn a successful spawn into a Lua failure. */
    static const char *const collider_keys[] = {"collider", "graph", "x", "y", "angle", "size", "fpg"};
    lua_createtable(L, 7, 0);
    for (size_t i = 0; i < sizeof(collider_keys) / sizeof(collider_keys[0]); ++i) {
        lua_pushstring(L, collider_keys[i]);
        lua_seti(L, -2, (lua_Integer)i + 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.collider_keys");
    luaL_newmetatable(L, "malvado.camera");
    lua_pushcfunction(L, camera_destroy);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    static const luaL_Reg camera_methods[] = {
        {"set", camera_set}, {"move", camera_move}, {"position", camera_position},
        {"follow", camera_follow}, {"destroy", camera_destroy}, {NULL, NULL}
    };
    luaL_setfuncs(L, camera_methods, 0);
    lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "malvado camera");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.graphic");
    lua_pushliteral(L, "malvado graphic");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.collision_world");
    lua_pushcfunction(L, world_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, world_add_rect);
    lua_setfield(L, -2, "add_rect");
    lua_pushcfunction(L, world_overlap);
    lua_setfield(L, -2, "overlap");
    lua_pushcfunction(L, world_sweep);
    lua_setfield(L, -2, "sweep");
    lua_pushcfunction(L, world_process_collision);
    lua_setfield(L, -2, "collision");
    lua_pushcfunction(L, world_process_move);
    lua_setfield(L, -2, "move");
    lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "malvado collision world");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.scroll");
    lua_pushliteral(L, "malvado scroll");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.region");
    lua_pushliteral(L, "malvado region");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.collision_shape");
    lua_pushliteral(L, "malvado collision shape");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
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
    luaL_newmetatable(L, "malvado.fpg_manifest");
    lua_pushcfunction(L, fpg_manifest_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    static const luaL_Reg manifest_methods[] = {
        {"count", fpg_manifest_count}, {"id_at", fpg_manifest_id_at},
        {"transparent_zero", fpg_manifest_transparent}, {"palette", fpg_manifest_palette},
        {"gamma", fpg_manifest_gamma}, {"sprite", fpg_manifest_sprite},
        {"point", fpg_manifest_point}, {"graph", graph_select}, {"animation", animation_select}, {NULL, NULL}
    };
    luaL_setfuncs(L, manifest_methods, 0);
    lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "malvado FPG manifest");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.fpg");
    lua_pushcfunction(L, fpg_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    static const luaL_Reg fpg_methods[] = {
        {"count", fpg_count}, {"id_at", fpg_id_at}, {"palette", fpg_palette}, {"gamma", fpg_gamma},
        {"sprite", fpg_sprite}, {"point", fpg_point}, {"rgba", fpg_rgba},
        {"graph", graph_select}, {"animation", animation_select}, {"image", fpg_image}, {"png", fpg_png}, {"json", fpg_json}, {NULL, NULL}
    };
    luaL_setfuncs(L, fpg_methods, 0);
    lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "malvado FPG");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.animation");
    lua_pushcfunction(L, animation_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, animation_sample);
    lua_setfield(L, -2, "sample");
    lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "malvado animation");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "malvado.atlas");
    lua_pushcfunction(L, atlas_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, graph_select);
    lua_setfield(L, -2, "graph");
    lua_pushcfunction(L, animation_select);
    lua_setfield(L, -2, "animation");
    lua_setfield(L, -2, "__index");
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
    lua_newtable(L);
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.cameras");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.playing_audio");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    luaL_newlib(L, functions);
    lua_newtable(L);
    static const luaL_Reg camera_functions[] = {
        {"create", camera_create}, {"set", camera_set}, {"move", camera_move},
        {"position", camera_position}, {"follow", camera_follow}, {NULL, NULL}
    };
    luaL_setfuncs(L, camera_functions, 0);
    lua_setfield(L, -2, "camera");
    lua_newtable(L);
    lua_pushinteger(L, MV_COLLISION_CIRCLE);
    lua_pushcclosure(L, graphic_create, 1);
    lua_setfield(L, -2, "circle");
    lua_pushinteger(L, MV_COLLISION_RECT);
    lua_pushcclosure(L, graphic_create, 1);
    lua_setfield(L, -2, "rect");
    lua_pushcfunction(L, region_create);
    lua_setfield(L, -2, "region");
    lua_pushcfunction(L, scroll_create);
    lua_setfield(L, -2, "scroll");
#ifdef MALVADO_BUILD_ASSETS
    lua_pushcfunction(L, graphic_sprite);
    lua_setfield(L, -2, "sprite");
    lua_pushcfunction(L, graph_select);
    lua_setfield(L, -2, "graph");
    lua_pushcfunction(L, animation_select);
    lua_setfield(L, -2, "animation");
    lua_pushcfunction(L, animation_elapsed);
    lua_setfield(L, -2, "animation_elapsed");
#endif
    /* Copy the closures, including their kind upvalues, rather than creating
     * separate constructors for the procedural interface. */
    static const char *const graphic_aliases[] = {"rect", "circle", "region", "scroll"};
    for (size_t i = 0; i < sizeof(graphic_aliases) / sizeof(graphic_aliases[0]); ++i) {
        lua_getfield(L, -1, graphic_aliases[i]);
        lua_setfield(L, -3, graphic_aliases[i]);
    }
    lua_setfield(L, -2, "graphics");
#ifdef MALVADO_BUILD_ASSETS
    lua_newtable(L);
    lua_pushcfunction(L, fpg_parse);
    lua_setfield(L, -2, "parse");
    lua_pushcfunction(L, fpg_manifest_parse);
    lua_setfield(L, -2, "parse_manifest");
    lua_setfield(L, -2, "fpg");
#endif
    lua_newtable(L);
    lua_pushcfunction(L, world_create);
    lua_setfield(L, -2, "create");
    lua_pushcfunction(L, world_move);
    lua_setfield(L, -2, "move");
    lua_setfield(L, -2, "world");
    lua_newtable(L);
    lua_pushinteger(L, MV_COLLISION_CIRCLE);
    lua_pushcclosure(L, collision_shape, 1);
    lua_setfield(L, -2, "circle");
    lua_pushinteger(L, MV_COLLISION_RECT);
    lua_pushcclosure(L, collision_shape, 1);
    lua_setfield(L, -2, "rect");
    lua_pushcfunction(L, collision_overlap);
    lua_setfield(L, -2, "overlap");
    lua_newtable(L);
    lua_pushcfunction(L, call_collision_namespace);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "collision");
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
    lua_pushcfunction(L, process_collision_first);
    lua_setfield(L, -2, "collision");
    lua_pushcfunction(L, process_collision_all);
    lua_setfield(L, -2, "collisions");
    lua_newtable(L);
    lua_pushcfunction(L, call_process_namespace);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "process");
    lua_getfield(L, -1, "key");
    lua_setfield(L, -2, "key_down");
    lua_newtable(L);
    static const char *const input_functions[] = {
        "key_down", "key_pressed", "key_released", "mouse_position", "mouse_button",
        "pad_connected", "pad_button",
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

static int
call_coroutine_operation(lua_State *L)
{
    lua_State *thread = lua_tothread(L, 1);
    if (thread) {
        for (size_t i = 0; i < engine.count; i++) {
            if (engine.items[i].alive && engine.items[i].thread == thread) {
                lua_pushboolean(L, 0);
                lua_pushliteral(L, "cannot control a managed process coroutine");
                return 2;
            }
        }
    }
    int arguments = lua_gettop(L);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    lua_call(L, arguments, LUA_MULTRET);
    return lua_gettop(L);
}

static void
protect_process_coroutines(lua_State *L)
{
    static const char *const operations[] = {"resume", "close"};
    lua_getglobal(L, "coroutine");
    for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++) {
        lua_getfield(L, -1, operations[i]);
        lua_pushcclosure(L, call_coroutine_operation, 1);
        lua_setfield(L, -2, operations[i]);
    }
    lua_pop(L, 1);
}

static int
initialize_lua(lua_State *L)
{
    luaL_openlibs(L);
    protect_process_coroutines(L);
    luaL_requiref(L, "malvado", open_module, 0);
    lua_pop(L, 1);
    luaL_requiref(L, "malvado.compat.legacy", open_legacy_module, 0);
    lua_pop(L, 1);
    return 0;
}

int
malvado_run(int argc, char **argv)
{
    if (engine.context || engine.L) {
        fprintf(stderr, "A Malvado runner is already active\n");
        return 2;
    }
    memset(&engine, 0, sizeof(engine));
    engine.main_camera.target_ref = LUA_NOREF;
    int index = 1;
    if (argc > 1 && strcmp(argv[1], "--headless") == 0) {
        engine.headless = 1;
        index++;
    }
    if (argc != index + 1) {
        fprintf(stderr, "Usage: malvado-run [--headless] game.lua\n");
        return 2;
    }
    (void)mv_draw_state_init(&engine.draw);
    engine.context = mv_context_create(60, 5);
    if (!engine.context) {
        fprintf(stderr, "Could not create Malvado context\n");
        return 1;
    }
    if (mv_context_runtime_id(engine.context, &engine.runtime_id) != MV_OK) {
        fprintf(stderr, "Could not identify Malvado context\n");
        mv_context_destroy(engine.context);
        engine.context = NULL;
        return 1;
    }
    engine.L = luaL_newstate();
    if (!engine.L) {
        fprintf(stderr, "Could not create Lua state\n");
        mv_context_destroy(engine.context);
        engine.context = NULL;
        return 1;
    }
    lua_pushcfunction(engine.L, initialize_lua);
    int status = lua_pcall(engine.L, 0, 0, 0);
    if (status == LUA_OK)
        status = luaL_loadfile(engine.L, argv[index]);
    if (status == LUA_OK)
        status = lua_pcall(engine.L, 0, 1, 0);
    if (status != LUA_OK) {
        const char *message = lua_type(engine.L, -1) == LUA_TSTRING ?
                              lua_tostring(engine.L, -1) : "non-string error";
        fprintf(stderr, "%s\n", message);
        engine.failed = 1;
    } else if (lua_istable(engine.L, -1)) {
        lua_pushcfunction(engine.L, start_declared_game);
        lua_pushvalue(engine.L, -2);
        if (lua_pcall(engine.L, 1, 0, 0) != LUA_OK) {
            const char *message = lua_type(engine.L, -1) == LUA_TSTRING ?
                                  lua_tostring(engine.L, -1) : "non-string error";
            fprintf(stderr, "%s\n", message);
            engine.failed = 1;
        }
    }
    mv_context_destroy(engine.context);
    engine.context = NULL;
    invalidate_dead_handles(engine.L);
    for (size_t i = 0; i < engine.count; i++)
        cleanup(&engine.items[i]);
    free(engine.items);
    engine.items = NULL;
    engine.count = 0;
    camera_stop_following(engine.L, &engine.main_camera);
    lua_close(engine.L);
    engine.L = NULL;
    if (engine.audio)
        mv_audio_close();
    if (engine.window)
        mv_window_close();
    return engine.failed ? 1 : 0;
}
