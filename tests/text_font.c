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
#include "malvado/platform.h"
#include "../src/core/process.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct MvFont { int references; };
static int fail_allocation, drawn, last_x;
void *test_text_font_malloc(size_t size) { return fail_allocation ? NULL : malloc(size); }
int mv_font_retain(MvFont *font) { if (!font) return 0; ++font->references; return 1; }
void mv_font_destroy(MvFont *font) { if (font) --font->references; }
int mv_font_text_width(const MvFont *font, const char *text, int size)
{
    assert(font && font->references > 0 && text && size == 20);
    return 10;
}
int mv_window_text_width(const char *text, int size) { assert(text && size == 20); return 10; }
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color)
{
    assert(!strcmp(text,"retained") && y == 4 && size == 20 && color.r == 1);
    last_x = x;
    ++drawn;
}
int mv_font_draw(const MvFont *font, const char *text, int x, int y, int size, MvColor color)
{
    assert(font && font->references > 0);
    mv_window_draw_text(text,x,y,size,color);
    return 1;
}
static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx; (void)id; (void)user; return MV_STEP_YIELD;
}
int main(void)
{
    MvContext *ctx = mv_context_create(60,4);
    uint64_t id;
    MvColor color = {1,2,3,255};
    struct MvFont font = {1};
    assert(mv_text_entry_create(ctx,NULL,"retained",20,4,1,&id) == MV_OK);
    assert(malvado_text_entries_draw(ctx) && !drawn);
    assert(mv_text_entry_font(ctx,id,&font,0,color) == MV_INVALID_ARGUMENT && font.references == 1);
    assert(mv_text_entry_font(ctx,999,&font,20,color) == MV_NOT_FOUND && font.references == 1);
    assert(mv_text_entry_font(ctx,id,&font,20,color) == MV_OK && font.references == 2);
    fail_allocation = 1;
    assert(mv_text_entry_font(ctx,id,NULL,20,color) == MV_OUT_OF_MEMORY && font.references == 2);
    fail_allocation = 0;
    assert(malvado_text_entries_draw(ctx) && drawn == 1 && last_x == 15);
    assert(mv_text_entry_font(ctx,id,&font,20,color) == MV_OK && font.references == 2);
    mv_font_destroy(&font);
    assert(font.references == 1);
    assert(mv_text_entry_update(ctx,id,"retained",20,4,2) == MV_OK);
    assert(malvado_text_entries_draw(ctx) && drawn == 2 && last_x == 10);
    assert(mv_text_entry_font_clear(ctx,id) == MV_OK && font.references == 0);
    assert(mv_text_entry_font_clear(ctx,id) == MV_OK && font.references == 0);
    assert(malvado_text_entries_draw(ctx) && drawn == 2);
    assert(mv_text_entry_font(ctx,id,NULL,20,color) == MV_OK);
    assert(malvado_text_entries_draw(ctx) && drawn == 3);
    MvProcessId pid;
    MvProcessRef owner;
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&pid) == MV_OK);
    assert(mv_process_ref_create(ctx,pid,&owner) == MV_OK);
    uint64_t owned;
    font.references = 1;
    assert(mv_text_entry_create(ctx,&owner,"retained",20,4,1,&owned) == MV_OK);
    assert(mv_text_entry_font(ctx,owned,&font,20,color) == MV_OK && font.references == 2);
    int killed;
    assert(mv_process_kill(ctx,owner,&killed) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK && font.references == 1);
    assert(mv_text_entry_font(ctx,id,&font,20,color) == MV_OK && font.references == 2);
    mv_context_destroy(ctx);
    assert(font.references == 1);
    mv_font_destroy(&font);
    return 0;
}
