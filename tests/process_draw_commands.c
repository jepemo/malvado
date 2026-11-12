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
#include "../src/core/process.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user;
    return MV_STEP_YIELD;
}

static void release(void *data) { ++*(int *)data; }

static void inspect(const MvStoredDraw *command, void *user)
{
    unsigned *index = user;
    if (*index == 0) {
        assert(command->kind == MV_STORED_BOX && command->width == 4 && command->color.r == 1);
    } else if (*index == 1) {
        assert(command->kind == MV_STORED_CIRCLE && command->radius == 2 && command->filled == 0);
    } else if (*index == 2) {
        assert(command->kind == MV_STORED_TEXT && !strcmp(command->text,"copied"));
        assert(command->size == -1);
    } else assert(command->kind == MV_STORED_CUSTOM);
    ++*index;
}

int main(void)
{
    MvContext *ctx = mv_context_create(60,4), *other = mv_context_create(60,4);
    MvProcessId id;
    MvProcessRef ref;
    assert(ctx && other);
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&id) == MV_OK);
    assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
    size_t count = 99;
    MvColor color = {1,2,3,255};
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 0);
    assert(mv_process_draw_box(ctx,ref,0,0,4,6,1,color) == MV_OK);
    assert(mv_process_draw_circle(ctx,ref,0,0,2,0,color) == MV_OK);
    char text[] = "copied";
    assert(mv_process_draw_text(ctx,ref,text,INT_MIN,INT_MAX,-1,color) == MV_OK);
    text[0] = 'X';
    int releases = 0;
    MvStoredDraw owned = {.kind = MV_STORED_CUSTOM, .data = &releases, .destroy = release};
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_OK);
    assert(mv_process_draw_box(ctx,ref,INT_MAX,0,1,1,1,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_box(ctx,ref,0,0,0,1,1,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_box(ctx,ref,0,0,1,1,2,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_circle(ctx,ref,INT_MIN,0,1,1,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_circle(ctx,ref,0,INT_MAX,1,1,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_circle(ctx,ref,0,0,-1,1,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_text(ctx,ref,NULL,0,0,20,color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_clear(other,ref) == MV_INVALID_ARGUMENT);
    count = 99;
    assert(mv_process_draw_count(other,ref,&count) == MV_INVALID_ARGUMENT && count == 99);
    assert(mv_process_draw_count(NULL,ref,&count) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_count(ctx,ref,NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_signal(ctx,id,MV_SIGNAL_SLEEP,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(mv_process_signal(ctx,id,MV_SIGNAL_FREEZE,0) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK);
    unsigned index = 0;
    assert(malvado_process_draw_visit(ctx,ref,inspect,&index) == MV_OK && index == 4);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 4);
    assert(mv_process_graphic_clear(ctx,ref) == MV_OK);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 4);
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_OK);
    int killed;
    assert(mv_process_kill(ctx,ref,&killed) == MV_OK && killed && releases == 1);
    count = 99;
    assert(mv_process_draw_count(ctx,ref,&count) == MV_NOT_FOUND && count == 99);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_NOT_FOUND && releases == 1);
    assert(mv_process_tick(ctx) == MV_OK && releases == 2);
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&id) == MV_OK);
    assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_OK);
    mv_context_destroy(ctx);
    assert(releases == 3);
    mv_context_destroy(other);
    return 0;
}
