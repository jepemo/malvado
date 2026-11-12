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
#include "runner/runtime.h"
#include "malvado/text.h"
#include <assert.h>
#include <string.h>

static void between_ticks(void *user)
{
    int *stage = user;
    MvContext *ctx = malvado_debug_context();
    const char *value;
    int x, y, alignment;
    if (mv_text_entry_read(ctx, 1, &value, &x, &y, &alignment) != MV_OK) return;
    if (*stage == 0 && !strcmp(value, "Lua")) {
        assert(x == 1 && y == 2 && alignment == 1);
        assert(mv_text_entry_update(ctx, 1, "Native", 3, 4, 2) == MV_OK);
        ++*stage;
    } else if (*stage == 1 && !strcmp(value, "Changed")) {
        assert(x == 3 && y == 4 && alignment == 2);
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
