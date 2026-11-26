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

#include "malvado/platform.h"
#include "../core/process.h"
#include <stdlib.h>

typedef struct {
    MvFont *font;
    int size;
    MvColor color;
    void *owner;
    MvResourceDestroy destroy;
} TextFont;

static void
release_font(void *data)
{
    TextFont *style = data;
    mv_font_destroy(style->font);
    if (style->destroy) style->destroy(style->owner);
    free(style);
}

static int
draw_aligned_text(const char *text, int x, int y, int alignment,
                  MvFont *font, int size, MvColor color)
{
    if (alignment != MV_TEXT_ALIGN_LEFT) {
        /* The built-in renderer also draws nonpositive sizes at its minimum. */
        int width = font ? mv_font_text_width(font, text, size) :
                           mv_window_text_width(text, size < 1 ? 1 : size);
        (void)mv_text_aligned_x(x, width, alignment, &x);
    }
    if (font)
        return mv_font_draw(font, text, x, y, size, color);
    mv_window_draw_text(text, x, y, size, color);
    return 1;
}

static int
draw_text(const void *data, const MvText *text)
{
    const TextFont *style = data;
    const char *value;
    int x, y, alignment;
    if (mv_text_read(text, &value, &x, &y, &alignment) != MV_OK) return 0;
    return draw_aligned_text(value, x, y, alignment, style->font, style->size, style->color);
}

void
malvado_draw_stored_text(const MvStoredDraw *command)
{
    const TextFont *style = command->data;
    if (style) {
        (void)draw_aligned_text(command->text, command->x, command->y, command->alignment,
            style->font, style->size, style->color);
    } else {
        (void)draw_aligned_text(command->text, command->x, command->y, command->alignment,
            NULL, command->size, command->color);
    }
}

MvResult
mv_process_draw_text_font_owned(MvContext *ctx, MvProcessRef process,
    const char *text, int x, int y, int size, int alignment, MvColor color, MvFont *font,
    void *owner, MvResourceDestroy release)
{
    if (!text || !font || size < 1 || size > 1024 ||
        alignment < MV_TEXT_ALIGN_LEFT || alignment > MV_TEXT_ALIGN_RIGHT ||
        (owner != NULL) != (release != NULL))
        return MV_INVALID_ARGUMENT;
    size_t count;
    MvResult result = mv_process_draw_count(ctx, process, &count);
    if (result != MV_OK) return result;
    TextFont *style = malloc(sizeof(*style));
    if (!style) return MV_OUT_OF_MEMORY;
    if (!mv_font_retain(font)) {
        free(style);
        return MV_OVERFLOW;
    }
    *style = (TextFont){font, size, color, owner, release};
    result = malvado_process_draw_owned(ctx, process, (MvStoredDraw){.kind = MV_STORED_TEXT,
        .x = x, .y = y, .alignment = alignment, .text = text,
        .data = style, .destroy = release_font});
    if (result != MV_OK) {
        style->destroy = NULL;
        release_font(style);
    }
    return result;
}

MvResult
mv_process_draw_text_font(MvContext *ctx, MvProcessRef process,
    const char *text, int x, int y, int size, int alignment, MvColor color, MvFont *font)
{
    return mv_process_draw_text_font_owned(ctx, process, text, x, y, size, alignment,
        color, font, NULL, NULL);
}

MvResult
mv_text_entry_font_owned(MvContext *ctx, uint64_t id, MvFont *font, int size, MvColor color,
    void *owner, MvResourceDestroy destroy)
{
    if ((owner && !destroy) || (!owner && destroy) || size < 1 || size > 1024)
        return MV_INVALID_ARGUMENT;
    const char *value;
    int x, y, alignment;
    MvResult result = mv_text_entry_read(ctx, id, &value, &x, &y, &alignment);
    if (result != MV_OK) return result;
    void *previous = NULL;
    result = malvado_text_entry_binding(ctx, id, draw_text, &previous);
    if (result != MV_OK) return result;
    TextFont *old = previous;
    if (!owner && old && !old->owner && old->font == font && old->size == size &&
        old->color.r == color.r && old->color.g == color.g &&
        old->color.b == color.b && old->color.a == color.a)
        return MV_OK;
    TextFont *style = malloc(sizeof(*style));
    if (!style) return MV_OUT_OF_MEMORY;
    if (font && !mv_font_retain(font)) {
        free(style);
        return MV_INVALID_ARGUMENT;
    }
    *style = (TextFont){font, size, color, owner, destroy};
    result = malvado_text_entry_bind(ctx, id, style, release_font, draw_text);
    if (result != MV_OK) {
        style->owner = NULL;
        style->destroy = NULL;
        release_font(style);
    }
    return result;
}

MvResult
mv_text_entry_font(MvContext *ctx, uint64_t id, MvFont *font, int size, MvColor color)
{
    return mv_text_entry_font_owned(ctx, id, font, size, color, NULL, NULL);
}

MvResult
mv_text_entry_font_clear(MvContext *ctx, uint64_t id)
{
    return malvado_text_entry_bind(ctx, id, NULL, NULL, NULL);
}
