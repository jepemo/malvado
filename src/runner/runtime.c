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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "raylib.h"
#include "rlgl.h"
#include "core/clock.h"
#include "core/schedule.h"
#include "runtime.h"

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
    int queue_ref;
    int draw_ref;
    unsigned queue_head, queue_tail, queue_count;
    lua_Integer parent_id;
    lua_Integer priority;
    ProcessState state;
    int first;
    int alive;
} Process;
typedef struct {
    lua_State *L;
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
    Color background;
} Engine;
static Engine engine;
static MalvadoDebugHook debug_hook;
static void *debug_user;
static MalvadoDebugHook debug_frame_hook;
static void *debug_frame_user;

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

int
malvado_debug_snapshot(MalvadoProcessInfo *out, size_t capacity,
                       size_t *total, int64_t *tick)
{
    if (!engine.L || engine.in_tick || !total || !tick || (capacity && !out))
        return 0;
    size_t count = 0;
    for (size_t i = 0; i < engine.count; i++) {
        Process *p = &engine.items[i];
        if (!p->alive || p->state == PROCESS_KILLED)
            continue;
        if (count < capacity) {
            static const char *names[] = {"running", "sleeping", "frozen", "killed"};
            out[count] = (MalvadoProcessInfo){
                p->id, p->parent_id, p->type_id, p->priority,
                (size_t)p->queue_count, names[p->state]
            };
        }
        count++;
    }
    *total = count;
    *tick = engine.tick_number;
    return 1;
}

int
malvado_debug_signal(int64_t id, const char *signal)
{
    if (!engine.L || engine.in_tick || !signal)
        return 0;
    Process *process = find_process((lua_Integer)id);
    if (!process)
        return 0;
    if (strcmp(signal, "sleep") == 0)
        process->state = PROCESS_SLEEPING;
    else if (strcmp(signal, "freeze") == 0)
        process->state = PROCESS_FROZEN;
    else if (strcmp(signal, "wakeup") == 0)
        process->state = PROCESS_RUNNING;
    else if (strcmp(signal, "kill") == 0)
        process->state = PROCESS_KILLED;
    else
        return 0;
    return 1;
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

/* Only living ancestors form the current tree; a dead parent orphans children. */
static int
is_descendant(Process *process, lua_Integer ancestor_id)
{
    lua_Integer parent_id = process->parent_id;
    while (parent_id) {
        if (parent_id == ancestor_id)
            return 1;
        Process *parent = find_process_slot(parent_id);
        if (!parent)
            return 0;
        parent_id = parent->parent_id;
    }
    return 0;
}

static int
receive_message(lua_State *L)
{
    lua_Integer id = lua_tointeger(L, lua_upvalueindex(1));
    Process *process = find_process(id);
    if (!process)
        return luaL_error(L, "unknown process id");
    if (process->queue_count == 0) {
        lua_pushnil(L);
        return 1;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->queue_ref);
    lua_geti(L, -1, process->queue_head);
    lua_pushnil(L);
    lua_seti(L, -3, process->queue_head);
    process->queue_head = process->queue_head % 1024 + 1;
    process->queue_count--;
    lua_remove(L, -2);
    return 1;
}

static int
send_message(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    luaL_checkany(L, 2);
    if (lua_isnil(L, 2))
        return luaL_error(L, "message must not be nil");
    Process *process = find_process(id);
    if (!process)
        return luaL_error(L, "unknown process id");
    if (process->queue_count == 1024)
        return luaL_error(L, "message queue is full");
    lua_rawgeti(L, LUA_REGISTRYINDEX, process->queue_ref);
    lua_pushvalue(L, 2);
    process->queue_tail = process->queue_tail % 1024 + 1;
    lua_seti(L, -2, process->queue_tail);
    process->queue_count++;
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
    Process *process = find_process(id);
    if (!process)
        return luaL_error(L, "unknown process id");
    process->state = state;
    return 0;
}

static int
signal_tree(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    ProcessState state = parse_signal(L, 2);
    if (!find_process(id))
        return luaL_error(L, "unknown process id");
    for (size_t i = 0; i < engine.count; i++) {
        Process *process = &engine.items[i];
        if (process->alive && process->state != PROCESS_KILLED &&
            (process->id == id || is_descendant(process, id)))
            process->state = state;
    }
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
    Process *process = find_process(id);
    if (!process)
        return luaL_error(L, "unknown process id");
    process->priority = priority;
    return 0;
}

static int
get_priority(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    Process *process = find_process(id);
    if (!process)
        return luaL_error(L, "unknown process id");
    lua_pushinteger(L, process->priority);
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
    Process *process = find_process(id);
    if (!process) {
        lua_pushnil(L);
        return 1;
    }
    const char *states[] = {"running", "sleeping", "frozen", "killed"};
    lua_pushstring(L, states[process->state]);
    return 1;
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
    lua_Integer id = ++engine.next_process_id;
    lua_pushinteger(L, id);
    lua_setfield(L, -2, "id");
    if (engine.active_id) {
        lua_pushinteger(L, engine.active_id);
        lua_setfield(L, -2, "parent");
    }
    lua_pushinteger(L, id);
    lua_pushcclosure(L, receive_message, 1);
    lua_setfield(L, -2, "recv");
    int self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_State *co = lua_newthread(L);
    int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_xmove(L, co, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref);
    lua_xmove(L, co, 1);
    lua_newtable(L);
    int queue_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_newtable(L);
    int draw_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    engine.items[engine.count++] = (Process) {
        .id = id,
        .type_id = lua_tointeger(L, lua_upvalueindex(2)),
        .thread = co,
        .thread_ref = thread_ref,
        .self_ref = self_ref,
        .queue_ref = queue_ref,
        .draw_ref = draw_ref,
        .queue_head = 1,
        .queue_tail = 0,
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
    lua_newtable(L);
    lua_Integer result = 0;
    for (size_t i = 0; i < engine.count; i++) {
        Process *process = &engine.items[i];
        if (process->alive && process->state != PROCESS_KILLED && process->type_id == type_id) {
            lua_pushinteger(L, process->id);
            lua_seti(L, -2, ++result);
        }
    }
    return 1;
}

static int
snapshot(lua_State *L)
{
    lua_newtable(L);
    lua_pushinteger(L, engine.tick_number);
    lua_setfield(L, -2, "tick");
    lua_newtable(L);
    lua_Integer result = 0;
    size_t count = engine.count;
    for (size_t i = 0; i < count; i++) {
        /* Lua allocation can run a finalizer that creates a process. */
        Process process = engine.items[i];
        Process *p = &process;
        if (!p->alive || p->state == PROCESS_KILLED)
            continue;
        lua_createtable(L, 0, 8);
        lua_pushinteger(L, p->id); lua_setfield(L, -2, "id");
        if (p->parent_id) {
            lua_pushinteger(L, p->parent_id); lua_setfield(L, -2, "parent");
        }
        lua_pushinteger(L, p->type_id); lua_setfield(L, -2, "type");
        lua_pushinteger(L, p->priority); lua_setfield(L, -2, "priority");
        lua_pushstring(L, state_name(p->state)); lua_setfield(L, -2, "status");
        lua_pushinteger(L, p->queue_count); lua_setfield(L, -2, "messages");
        lua_rawgeti(L, LUA_REGISTRYINDEX, p->draw_ref);
        lua_Integer draw_count = (lua_Integer)lua_rawlen(L, -1);
        lua_pop(L, 1);
        lua_pushinteger(L, draw_count); lua_setfield(L, -2, "draw_commands");
        lua_pushboolean(L, p->state != PROCESS_SLEEPING); lua_setfield(L, -2, "visible");
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
    engine.background = (Color) {
        r, g, b, 255
    };
    return 0;
}

static int
key(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    if (strcmp(s, "escape") != 0)
        return luaL_error(L, "unsupported key: %s", s);
    lua_pushboolean(L, !engine.headless && IsKeyDown(KEY_ESCAPE));
    return 1;
}

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
        Color c = WHITE;
        unsigned char *channels[] = {&c.r, &c.g, &c.b, &c.a};
        for (int i = 0; i < 4; ++i) {
            lua_geti(L, -1, i + 1);
            *channels[i] = (unsigned char)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        DrawText(text, x, y, size, c);
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
    int queue_ref = p->queue_ref;
    int draw_ref = p->draw_ref;
    p->alive = 0;
    int status = lua_resetthread(thread);
    if (status != LUA_OK) {
        fprintf(stderr, "Error closing process: %s\n", lua_tostring(thread, -1));
        engine.failed = 1;
    }
    luaL_unref(engine.L, LUA_REGISTRYINDEX, thread_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, self_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, queue_ref);
    luaL_unref(engine.L, LUA_REGISTRYINDEX, draw_ref);
}

static int
tick(void)
{
    lua_State *L = engine.L;
    engine.in_tick = 1;
    engine.tick_number++;
    size_t count = engine.count;
    if (count > SIZE_MAX / sizeof(MalvadoScheduleItem)) {
        fprintf(stderr, "Too many processes to schedule\n");
        engine.in_tick = 0;
        engine.failed = 1;
        return 0;
    }
    MalvadoScheduleItem *order = count ? malloc(count * sizeof(*order)) : NULL;
    if (count && !order) {
        fprintf(stderr, "Out of memory scheduling processes\n");
        engine.in_tick = 0;
        engine.failed = 1;
        return 0;
    }
    for (size_t i = 0; i < count; i++)
        order[i] = (MalvadoScheduleItem){i, engine.items[i].priority};
    malvado_schedule_sort(order, count);
    for (size_t position = 0; position < count; position++) {
        size_t i = order[position].slot;
        if (!engine.items[i].alive || engine.items[i].state != PROCESS_RUNNING)
            continue;
        lua_State *co = engine.items[i].thread;
        /* Replace only this process's commands; frozen processes keep theirs. */
        lua_newtable(L);
        lua_rawseti(L, LUA_REGISTRYINDEX, engine.items[i].draw_ref);
        int first = engine.items[i].first;
        engine.items[i].first = 0;
        engine.active = 1;
        engine.active_thread = co;
        engine.active_id = engine.items[i].id;
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
            cleanup(&engine.items[i]);
            free(order);
            engine.in_tick = 0;
            return 0;
        }
        if (results)
            lua_pop(co, results);
        if (status == LUA_OK)
            cleanup(&engine.items[i]);
    }
    free(order);
    /* A running coroutine cannot be reset until lua_resume has returned. */
    for (size_t i = 0; i < engine.count; i++) {
        if (engine.items[i].alive && engine.items[i].state == PROCESS_KILLED)
            cleanup(&engine.items[i]);
    }

    /* Drop dead slots only after all callbacks and resumptions have finished. */
    size_t write = 0;
    for (size_t i = 0; i < engine.count; i++) {
        if (!engine.items[i].alive)
            continue;
        if (engine.items[i].parent_id && !find_process(engine.items[i].parent_id))
            engine.items[i].parent_id = 0;
        if (write != i)
            engine.items[write] = engine.items[i];
        write++;
    }
    engine.count = write;
    engine.in_tick = 0;
    return write != 0;
}

static void
render(void)
{
    lua_State *L = engine.L;
    BeginDrawing();
    ClearBackground(engine.background);
    for (size_t p = 0; p < engine.count; p++) {
        Process *process = &engine.items[p];
        if (!process->alive || process->state == PROCESS_SLEEPING || process->state == PROCESS_KILLED)
            continue;
        lua_rawgeti(L, LUA_REGISTRYINDEX, process->draw_ref);
        for (lua_Integer i = 1; i <= (lua_Integer)lua_rawlen(L, -1); i++) {
            lua_geti(L, -1, i);
            lua_geti(L, -1, 1);
            const char *text = lua_tostring(L, -1);
            lua_geti(L, -2, 2);
            int x = lua_tointeger(L, -1);
            lua_geti(L, -3, 3);
            int y = lua_tointeger(L, -1);
            lua_geti(L, -4, 4);
            int size = lua_tointeger(L, -1);
            DrawText(text, x, y, size, WHITE);
            lua_pop(L, 5);
        }
        lua_pop(L, 1);
    }
    render_texts(L);
    if (debug_frame_hook) {
        rlDrawRenderBatchActive();
        debug_frame_hook(debug_frame_user);
    }
    EndDrawing();
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
        InitWindow(w, h, title);
        engine.window = 1;
        SetExitKey(KEY_NULL);
        SetTargetFPS(60);
    }
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        engine.failed = 1;
        return lua_error(L);
    }
    MalvadoClock clock;
    malvado_clock_init(&clock, 60.0, 5);
    double last_time = engine.headless ? 0 : GetTime();
    int alive = tick();
    if (debug_hook)
        debug_hook(debug_user);
    while (!engine.failed && alive && (engine.headless || !WindowShouldClose())) {
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
                last_time = GetTime();
            else if (engine.paused && !engine.step_requested)
                WaitTime(0.001);
            continue;
        }
        if (engine.headless) {
            alive = tick();
            if (debug_hook)
                debug_hook(debug_user);
        } else {
            render();
            double now = GetTime();
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
        {"draw_text", text},
        {"font", font},
        {"write", write_text},
        {"change_text", change_text},
        {"delete_text", delete_text},
        {NULL, NULL}
    };
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "malvado.texts");
    luaL_newlib(L, functions);
    return 1;
}

int
malvado_run(int argc, char **argv)
{
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
    engine.background = BLACK;
    engine.L = luaL_newstate();
    if (!engine.L)
        return 1;
    luaL_openlibs(engine.L);
    luaL_requiref(engine.L, "malvado", open_module, 0);
    lua_pop(engine.L, 1);
    if (luaL_loadfile(engine.L, argv[index]) != LUA_OK || lua_pcall(engine.L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(engine.L, -1));
        engine.failed = 1;
    }
    for (size_t i = 0; i < engine.count; i++)
        cleanup(&engine.items[i]);
    if (engine.window)
        CloseWindow();
    free(engine.items);
    lua_close(engine.L);
    engine.L = NULL;
    return engine.failed ? 1 : 0;
}
