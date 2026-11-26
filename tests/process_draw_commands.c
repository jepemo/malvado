/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
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
#include <float.h>
#include <limits.h>
#include <math.h>
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
        assert(command->kind == MV_STORED_LINE && command->thickness == 1.5f);
        assert(command->x == INT_MIN && command->y == 2 && command->x2 == INT_MAX && command->y2 == 3);
        assert(command->color.a == 127);
    } else if (*index == 3) {
        assert(command->kind == MV_STORED_TEXT && !strcmp(command->text,"copied"));
        assert(command->size == -1 && command->alignment == MV_TEXT_ALIGN_LEFT);
    } else if (*index == 4) {
        assert(command->kind == MV_STORED_TEXT && !strcmp(command->text, "line\ntext"));
        assert(command->size == 20 && command->alignment == MV_TEXT_ALIGN_RIGHT);
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
    assert(mv_process_draw_line(ctx,ref,INT_MIN,2,INT_MAX,3,1.5,
        (MvColor){1,2,3,127}) == MV_OK);
    const double invalid[] = {0, -1, NAN, INFINITY, FLT_MIN / 2.0, FLT_MAX};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
        assert(mv_process_draw_line(ctx,ref,0,0,0,0,invalid[i],color) == MV_INVALID_ARGUMENT);
    assert(mv_process_draw_line(other,ref,0,0,1,1,1,color) == MV_INVALID_ARGUMENT);
    char text[] = "copied";
    assert(mv_process_draw_text(ctx,ref,text,INT_MIN,INT_MAX,-1,color) == MV_OK);
    text[0] = 'X';
    assert(mv_process_draw_text_aligned(ctx,ref,"line\ntext",10,20,20,
        MV_TEXT_ALIGN_RIGHT,color) == MV_OK);
    assert(mv_process_draw_text_aligned(ctx,ref,"invalid",0,0,20,-1,color) == MV_INVALID_ARGUMENT);
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
    assert(malvado_process_draw_visit(ctx,ref,inspect,&index) == MV_OK && index == 6);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 6);
    assert(mv_process_graphic_clear(ctx,ref) == MV_OK);
    assert(mv_process_draw_count(ctx,ref,&count) == MV_OK && count == 6);
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(mv_process_draw_clear(ctx,ref) == MV_OK && releases == 1);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_OK);
    int killed;
    assert(mv_process_kill(ctx,ref,&killed) == MV_OK && killed && releases == 1);
    count = 99;
    assert(mv_process_draw_count(ctx,ref,&count) == MV_NOT_FOUND && count == 99);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_NOT_FOUND && releases == 1);
    assert(mv_process_draw_line(ctx,ref,0,0,1,1,1,color) == MV_NOT_FOUND);
    assert(mv_process_tick(ctx) == MV_OK && releases == 2);
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&id) == MV_OK);
    assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
    assert(malvado_process_draw_owned(ctx,ref,owned) == MV_OK);
    mv_context_destroy(ctx);
    assert(releases == 3);
    mv_context_destroy(other);
    return 0;
}
