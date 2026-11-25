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

#ifndef MALVADO_GAME_PATH_H
#define MALVADO_GAME_PATH_H

#include <stddef.h>
#include "malvado/malvado.h"

char *malvado_path_current_directory(void);
char *malvado_path_launcher_directory(void);
char *malvado_path_executable_directory(void);
char *malvado_path_absolute(const char *path);
char *malvado_path_parent(const char *absolute_path);
MvResult malvado_path_join(const char *base, const char *path, char *out,
                           size_t capacity, size_t *required);

char *malvado_asset_path(const MvContext *ctx, const char *path);

#endif
