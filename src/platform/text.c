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

#include "malvado/platform.h"
#include "../core/process.h"
#include <stdlib.h>

typedef struct {
    MvFont *font;
    int size;
    MvColor color;
} TextFont;

static void
release_font(void *data)
{
    TextFont *style = data;
    mv_font_destroy(style->font);
    free(style);
}

static int
draw_text(const void *data, const MvText *text)
{
    const TextFont *style = data;
    const char *value;
    int x, y, alignment;
    if (mv_text_read(text, &value, &x, &y, &alignment) != MV_OK) return 0;
    int width = style->font ? mv_font_text_width(style->font, value, style->size) :
                             mv_window_text_width(value, style->size);
    (void)mv_text_aligned_x(x, width, alignment, &x);
    if (style->font)
        return mv_font_draw(style->font, value, x, y, style->size, style->color);
    mv_window_draw_text(value, x, y, style->size, style->color);
    return 1;
}

MvResult
mv_text_entry_font(MvContext *ctx, uint64_t id, MvFont *font, int size, MvColor color)
{
    if (size < 1 || size > 1024) return MV_INVALID_ARGUMENT;
    const char *value;
    int x, y, alignment;
    MvResult result = mv_text_entry_read(ctx, id, &value, &x, &y, &alignment);
    if (result != MV_OK) return result;
    void *previous = NULL;
    result = malvado_text_entry_binding(ctx, id, draw_text, &previous);
    if (result != MV_OK) return result;
    TextFont *old = previous;
    if (old && old->font == font && old->size == size && old->color.r == color.r &&
        old->color.g == color.g && old->color.b == color.b && old->color.a == color.a)
        return MV_OK;
    TextFont *style = malloc(sizeof(*style));
    if (!style) return MV_OUT_OF_MEMORY;
    if (font && !mv_font_retain(font)) {
        free(style);
        return MV_INVALID_ARGUMENT;
    }
    *style = (TextFont){font, size, color};
    result = malvado_text_entry_bind(ctx, id, style, release_font, draw_text);
    if (result != MV_OK) release_font(style);
    return result;
}

MvResult
mv_text_entry_font_clear(MvContext *ctx, uint64_t id)
{
    return malvado_text_entry_bind(ctx, id, NULL, NULL, NULL);
}
