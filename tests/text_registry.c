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
#include "malvado/text.h"
#include "../src/core/process.h"
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

static int fail_allocation;
void *test_text_registry_calloc(size_t count, size_t size)
{
    return fail_allocation ? NULL : calloc(count, size);
}

static MvStepResult wait_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}
static MvStepResult done(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_DONE;
}
static void check(MvContext *ctx, uint64_t id, const char *expected)
{
    const char *text;
    int x, y, align;
    assert(mv_text_entry_read(ctx, id, &text, &x, &y, &align) == MV_OK);
    assert(!strcmp(text, expected));
    assert(x == INT_MIN && y == INT_MAX && align == 1);
}
typedef struct { int drawn, released; } Style;
static void release_style(void *data) { ++((Style *)data)->released; }
static int draw_style(const void *data, const MvText *text)
{
    Style *style = (Style *)data;
    const char *value;
    int x, y, alignment;
    assert(mv_text_read(text, &value, &x, &y, &alignment) == MV_OK);
    ++style->drawn;
    return 1;
}
typedef struct { MvContext *ctx; uint64_t id; int called; } Cleanup;
static void cleanup(void *user)
{
    Cleanup *data = user;
    check(data->ctx, data->id, "updated");
    ++data->called;
}
int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *foreign = mv_context_create(60, 4);
    MvProcessId pid, other;
    MvProcessRef owner, wrong;
    Cleanup data = {.ctx = ctx};
    assert(mv_process_spawn(ctx, 0, 0, wait_step, cleanup, &data, &pid) == MV_OK);
    assert(mv_process_ref_create(ctx, pid, &owner) == MV_OK);
    assert(mv_process_spawn(foreign, 0, 0, wait_step, NULL, NULL, &other) == MV_OK);
    assert(mv_process_ref_create(foreign, other, &wrong) == MV_OK);
    uint64_t host = 0, owned = 0, sentinel = 99;
    assert(mv_text_entry_create(NULL, NULL, "x", 0, 0, 0, &sentinel) == MV_INVALID_ARGUMENT && sentinel == 99);
    assert(mv_text_entry_create(ctx, &wrong, "x", 0, 0, 0, &sentinel) == MV_INVALID_ARGUMENT && sentinel == 99);
    assert(mv_text_entry_create(ctx, NULL, "x", 0, 0, 3, &sentinel) == MV_INVALID_ARGUMENT && sentinel == 99);
    fail_allocation = 1;
    assert(mv_text_entry_create(ctx, NULL, "oom", 0, 0, 0, &sentinel) == MV_OUT_OF_MEMORY && sentinel == 99);
    fail_allocation = 0;
    char source[] = "host";
    assert(mv_text_entry_create(ctx, NULL, source, INT_MIN, INT_MAX, 1, &host) == MV_OK);
    source[0] = 'X';
    check(ctx, host, "host");
    assert(mv_text_entry_create(ctx, &owner, "owned", INT_MIN, INT_MAX, 1, &owned) == MV_OK);
    Style host_style = {0}, owner_style = {0};
    assert(malvado_text_entry_bind(ctx, host, &host_style, release_style, NULL) == MV_INVALID_ARGUMENT);
    assert(malvado_text_entry_bind(ctx, 999, &host_style, release_style, draw_style) == MV_NOT_FOUND);
    assert(!host_style.released);
    assert(malvado_text_entry_bind(ctx, host, &host_style, release_style, draw_style) == MV_OK);
    assert(malvado_text_entry_bind(ctx, owned, &owner_style, release_style, draw_style) == MV_OK);
    assert(malvado_text_entries_draw(ctx) && host_style.drawn == 1 && owner_style.drawn == 1);
    assert(malvado_text_entry_bind(ctx, host, NULL, NULL, NULL) == MV_OK && host_style.released == 1);
    assert(malvado_text_entry_bind(ctx, host, NULL, NULL, NULL) == MV_OK && host_style.released == 1);
    assert(malvado_text_entry_bind(ctx, host, &host_style, release_style, draw_style) == MV_OK);
    data.id = owned;
    assert(host == 1 && owned == 2);
    size_t count = 99;
    uint64_t ids[2] = {99, 99};
    assert(mv_text_entries(ctx, NULL, 0, &count) == MV_OK && count == 2);
    assert(mv_text_entries(ctx, ids, 1, &count) == MV_OVERFLOW && count == 2 && ids[0] == 99);
    assert(mv_text_entries(ctx, ids, 2, &count) == MV_OK && ids[0] == host && ids[1] == owned);
    assert(mv_text_entries(ctx, NULL, 1, &count) == MV_INVALID_ARGUMENT);
    assert(mv_text_entry_update(ctx, owned, NULL, 0, 0, 0) == MV_INVALID_ARGUMENT);
    check(ctx, owned, "owned");
    assert(mv_text_entry_update(ctx, owned, "updated", INT_MIN, INT_MAX, 1) == MV_OK);
    check(ctx, owned, "updated");
    assert(mv_process_signal(ctx, pid, MV_SIGNAL_FREEZE, 0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    check(ctx, owned, "updated");
    assert(mv_process_signal(ctx, pid, MV_SIGNAL_SLEEP, 0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    check(ctx, owned, "updated");
    int killed;
    assert(mv_process_kill(ctx, owner, &killed) == MV_OK);
    check(ctx, owned, "updated");
    assert(mv_process_tick(ctx) == MV_OK);
    assert(data.called == 1 && owner_style.released == 1);
    assert(mv_text_entry_remove(ctx, owned) == MV_NOT_FOUND);
    assert(mv_text_entry_create(ctx, &owner, "dead", 0, 0, 0, &sentinel) == MV_NOT_FOUND && sentinel == 99);
    check(ctx, host, "host");
    assert(mv_process_spawn(ctx, 0, 0, done, NULL, NULL, &pid) == MV_OK);
    assert(mv_process_ref_create(ctx, pid, &owner) == MV_OK);
    assert(mv_text_entry_create(ctx, &owner, "return", 0, 0, 0, &owned) == MV_OK && owned == 3);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_text_entry_remove(ctx, owned) == MV_NOT_FOUND);
    assert(mv_text_entry_remove(ctx, host) == MV_OK && host_style.released == 2);
    assert(mv_text_entry_remove(ctx, host) == MV_NOT_FOUND);
    assert(mv_text_entry_remove(ctx, 0) == MV_NOT_FOUND);
    assert(mv_text_entries(ctx, ids, 2, &count) == MV_OK && count == 0);
    assert(mv_text_entry_create(ctx, NULL, "shutdown", 0, 0, 0, &owned) == MV_OK && owned == 4);
    assert(malvado_text_entry_bind(ctx, owned, &host_style, release_style, draw_style) == MV_OK);
    mv_context_destroy(foreign);
    mv_context_destroy(ctx);
    assert(host_style.released == 3);
    return 0;
}
