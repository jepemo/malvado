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

#include "malvado/text.h"
#include <stdlib.h>
#include <string.h>

struct MvText {
    char *value;
    int x, y, alignment;
};

MvResult
mv_text_update(MvText *text, const char *value, int x, int y, int alignment)
{
    if (!text || !value || alignment < 0 || alignment > 2)
        return MV_INVALID_ARGUMENT;
    size_t length = strlen(value);
    if (length == SIZE_MAX)
        return MV_OUT_OF_MEMORY;
    char *copy = malloc(length + 1);
    if (!copy)
        return MV_OUT_OF_MEMORY;
    memcpy(copy, value, length + 1);
    free(text->value);
    *text = (MvText){copy, x, y, alignment};
    return MV_OK;
}

MvResult
mv_text_create(const char *value, int x, int y, int alignment, MvText **out)
{
    if (!out || !value || alignment < 0 || alignment > 2)
        return MV_INVALID_ARGUMENT;
    MvText *text = calloc(1, sizeof(*text));
    if (!text)
        return MV_OUT_OF_MEMORY;
    MvResult result = mv_text_update(text, value, x, y, alignment);
    if (result != MV_OK) {
        free(text);
        return result;
    }
    *out = text;
    return MV_OK;
}

MvResult
mv_text_read(const MvText *text, const char **value, int *x, int *y, int *alignment)
{
    if (!text || !value || !x || !y || !alignment)
        return MV_INVALID_ARGUMENT;
    *value = text->value;
    *x = text->x;
    *y = text->y;
    *alignment = text->alignment;
    return MV_OK;
}

void
mv_text_destroy(MvText *text)
{
    if (!text)
        return;
    free(text->value);
    free(text);
}
