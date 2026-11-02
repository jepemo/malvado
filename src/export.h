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
#ifndef MALVADO_EXPORT_H
#define MALVADO_EXPORT_H
#include <stddef.h>
int malvado_export(const char *output, int native, int app);
int malvado_export_default(int native, int app);
/* Return 2 if both launchers and required licenses are usable, 1 if they are
 * missing, or 0 if this build cannot export games. */
int malvado_export_status(void);
int malvado_sdk_root(char *path, size_t size);
#endif
