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

#include <string.h>
#include "malvado/platform.h"

int
mv_key_from_name(const char *name, MvKey *key)
{
    static const struct { const char *name; MvKey key; } named[] = {
        {"escape", MV_KEY_ESCAPE}, {"space", MV_KEY_SPACE},
        {"left", MV_KEY_LEFT}, {"right", MV_KEY_RIGHT},
        {"up", MV_KEY_UP}, {"down", MV_KEY_DOWN},
        {"enter", MV_KEY_ENTER}, {"tab", MV_KEY_TAB}, {"backspace", MV_KEY_BACKSPACE}
    };
    if (!name || !key || !name[0])
        return 0;
    if (!name[1]) {
        if (name[0] >= 'a' && name[0] <= 'z') {
            *key = (MvKey)(MV_KEY_A + name[0] - 'a');
            return 1;
        }
        if (name[0] >= '0' && name[0] <= '9') {
            *key = (MvKey)(MV_KEY_0 + name[0] - '0');
            return 1;
        }
    }
    for (size_t i = 0; i < sizeof(named) / sizeof(named[0]); ++i) {
        if (strcmp(name, named[i].name) == 0) {
            *key = named[i].key;
            return 1;
        }
    }
    return 0;
}

int
mv_mouse_button_from_name(const char *name, MvMouseButton *button)
{
    if (!name || !button)
        return 0;
    if (strcmp(name, "left") == 0)
        *button = MV_MOUSE_LEFT;
    else if (strcmp(name, "middle") == 0)
        *button = MV_MOUSE_MIDDLE;
    else if (strcmp(name, "right") == 0)
        *button = MV_MOUSE_RIGHT;
    else
        return 0;
    return 1;
}
