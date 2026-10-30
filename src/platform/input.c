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

size_t
mv_utf8_encode(uint32_t codepoint, char output[4])
{
    if (!output || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff))
        return 0;
    if (codepoint <= 0x7f) {
        output[0] = (char)codepoint;
        return 1;
    }
    if (codepoint <= 0x7ff) {
        output[0] = (char)(0xc0 | (codepoint >> 6));
        output[1] = (char)(0x80 | (codepoint & 0x3f));
        return 2;
    }
    if (codepoint <= 0xffff) {
        output[0] = (char)(0xe0 | (codepoint >> 12));
        output[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        output[2] = (char)(0x80 | (codepoint & 0x3f));
        return 3;
    }
    output[0] = (char)(0xf0 | (codepoint >> 18));
    output[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
    output[2] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
    output[3] = (char)(0x80 | (codepoint & 0x3f));
    return 4;
}
