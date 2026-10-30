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

#include <assert.h>
#include <stddef.h>
#include "malvado/platform.h"

int
main(void)
{
    static const struct { const char *name; MvKey key; } cases[] = {
        {"escape", MV_KEY_ESCAPE}, {"space", MV_KEY_SPACE},
        {"left", MV_KEY_LEFT}, {"right", MV_KEY_RIGHT},
        {"up", MV_KEY_UP}, {"down", MV_KEY_DOWN},
        {"enter", MV_KEY_ENTER}, {"tab", MV_KEY_TAB}, {"backspace", MV_KEY_BACKSPACE},
        {"a", MV_KEY_A}, {"z", MV_KEY_Z}, {"0", MV_KEY_0}, {"9", MV_KEY_9}
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        MvKey key = MV_KEY_SPACE;
        assert(mv_key_from_name(cases[i].name, &key) && key == cases[i].key);
        assert(!mv_key_down(key));
    }
    MvKey unchanged = MV_KEY_A;
    assert(!mv_key_from_name("A", &unchanged) && unchanged == MV_KEY_A);
    assert(!mv_key_from_name("unknown", &unchanged) && unchanged == MV_KEY_A);
    assert(!mv_key_from_name(NULL, &unchanged));
    assert(!mv_key_from_name("left", NULL));
    assert(!mv_key_down((MvKey)-1));
    return 0;
}
