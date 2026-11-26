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
#include "runner/runtime.h"
#include "core/process.h"
#include "malvado/text.h"
#include <assert.h>
#include <string.h>

static void
check_draw_text(const MvStoredDraw *command, void *user)
{
    size_t *index = user;
    const char *values[] = {"", "one\ntwo", "override"};
    const int sizes[] = {20, 24, 30};
    assert(*index < 3 && command->kind == MV_STORED_TEXT);
    assert(!strcmp(command->text, values[*index]));
    assert(command->size == sizes[*index] && command->alignment == (int)*index);
    assert(command->color.r == 255 && command->color.g == 255 &&
           command->color.b == 255 && command->color.a == 255);
    ++*index;
}

static void between_ticks(void *user)
{
    int *stage = user;
    MvContext *ctx = malvado_debug_context();
    const char *value;
    int x, y, alignment;
    if (mv_text_entry_read(ctx, 1, &value, &x, &y, &alignment) != MV_OK) return;
    if (*stage == 0 && !strcmp(value, "Lua")) {
        assert(x == 1 && y == 2 && alignment == MV_TEXT_ALIGN_CENTER);
        MvProcessRef ref;
        size_t draws = 0, texts = 0;
        assert(mv_process_ref_create(ctx, 1, &ref) == MV_OK);
        assert(malvado_process_draw_visit(ctx, ref, check_draw_text, &draws) == MV_OK && draws == 3);
        assert(mv_text_entries(ctx, NULL, 0, &texts) == MV_OK && texts == 1);
        assert(mv_text_entry_update(ctx, 1, "Native", 3, 4, 2) == MV_OK);
        ++*stage;
    } else if (*stage == 1 && !strcmp(value, "Changed")) {
        assert(x == 3 && y == 4 && alignment == MV_TEXT_ALIGN_RIGHT);
        MvProcessRef ref;
        size_t draws;
        assert(mv_process_ref_create(ctx, 1, &ref) == MV_OK);
        assert(mv_process_draw_count(ctx, ref, &draws) == MV_OK && draws == 0);
        assert(mv_text_entry_remove(ctx, 1) == MV_OK);
        ++*stage;
    }
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    int stage = 0;
    char *args[] = {"malvado-run", "--headless", argv[1]};
    malvado_debug_set_hook(between_ticks, &stage);
    assert(malvado_run(3, args) == 0);
    malvado_debug_set_hook(NULL, NULL);
    assert(stage == 2);
    return 0;
}
