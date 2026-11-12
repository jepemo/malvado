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
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

static int fail_allocation;
static int fail_copy;
void *test_text_malloc(size_t size) { return (fail_allocation || fail_copy) ? NULL : malloc(size); }
void *test_text_calloc(size_t count, size_t size) { return fail_allocation ? NULL : calloc(count, size); }

int main(void)
{
    MvText *text = NULL;
    assert(mv_text_create(NULL, 0, 0, 0, &text) == MV_INVALID_ARGUMENT && !text);
    assert(mv_text_create("a", 0, 0, 3, &text) == MV_INVALID_ARGUMENT && !text);
    assert(mv_text_create("a", 0, 0, 0, NULL) == MV_INVALID_ARGUMENT);
    fail_allocation = 1;
    assert(mv_text_create("a", 0, 0, 0, &text) == MV_OUT_OF_MEMORY && !text);
    fail_allocation = 0;
    fail_copy = 1;
    assert(mv_text_create("a", 0, 0, 0, &text) == MV_OUT_OF_MEMORY && !text);
    fail_copy = 0;
    char source[] = "copied";
    assert(mv_text_create(source, INT_MIN, INT_MAX, 1, &text) == MV_OK);
    source[0] = 'X';
    const char *value = NULL;
    int x = 0, y = 0, alignment = 0;
    assert(mv_text_read(text, &value, &x, &y, &alignment) == MV_OK);
    assert(!strcmp(value, "copied") && x == INT_MIN && y == INT_MAX && alignment == 1);
    assert(mv_text_read(text, &value, &x, &y, NULL) == MV_INVALID_ARGUMENT);
    fail_allocation = 1;
    assert(mv_text_update(text, "replacement", 1, 2, 0) == MV_OUT_OF_MEMORY);
    fail_allocation = 0;
    assert(mv_text_update(text, "bad", 1, 2, -1) == MV_INVALID_ARGUMENT);
    assert(mv_text_update(text, NULL, 1, 2, 0) == MV_INVALID_ARGUMENT);
    assert(mv_text_read(text, &value, &x, &y, &alignment) == MV_OK);
    assert(!strcmp(value, "copied") && x == INT_MIN && y == INT_MAX && alignment == 1);
    assert(mv_text_update(text, value + 2, 3, 4, 2) == MV_OK);
    assert(mv_text_read(text, &value, &x, &y, &alignment) == MV_OK);
    assert(!strcmp(value, "pied") && x == 3 && y == 4 && alignment == 2);
    assert(mv_text_update(text, "", 0, 0, 0) == MV_OK);
    assert(mv_text_read(text, &value, &x, &y, &alignment) == MV_OK && !*value);
    mv_text_destroy(text);
    mv_text_destroy(NULL);
    assert(mv_text_update(NULL, "a", 0, 0, 0) == MV_INVALID_ARGUMENT);
    assert(mv_text_read(NULL, &value, &x, &y, &alignment) == MV_INVALID_ARGUMENT);
    return 0;
}
