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
#include <stdio.h>
_Static_assert(_Generic(&mv_window_open, int (*)(int, int, const char *): 1, default: 0), "mv_window_open signature");
_Static_assert(_Generic(&mv_window_set_title, int (*)(const char *): 1, default: 0), "mv_window_set_title signature");
_Static_assert(_Generic(&mv_window_size, int (*)(int *, int *): 1, default: 0), "mv_window_size signature");
_Static_assert(_Generic(&mv_window_close, void (*)(void): 1, default: 0), "mv_window_close signature");
_Static_assert(_Generic(&mv_window_should_close, int (*)(void): 1, default: 0), "mv_window_should_close signature");
_Static_assert(_Generic(&mv_window_begin, void (*)(MvColor): 1, default: 0), "mv_window_begin signature");
_Static_assert(_Generic(&mv_window_end, void (*)(void): 1, default: 0), "mv_window_end signature");
int main(void)
{
    printf("%zu %zu ", sizeof(MvColor), _Alignof(MvColor));
    printf("%zu ", offsetof(MvColor, r));
    printf("%zu ", offsetof(MvColor, g));
    printf("%zu ", offsetof(MvColor, b));
    printf("%zu ", offsetof(MvColor, a));
    return 0;
}
