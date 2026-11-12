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

#ifndef MALVADO_TEXT_H
#define MALVADO_TEXT_H
#include "malvado/malvado.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct MvText MvText;
/* Owned persistent text content and screen position, independent of a backend,
 * font or process. Strings are copied. Alignment: 0 left, 1 center, 2 right.
 * Create writes out only on success. Update replaces all fields atomically.
 * NULL arguments/unknown alignment return MV_INVALID_ARGUMENT; allocation
 * failure returns MV_OUT_OF_MEMORY. An update may use its own borrowed string.
 * Destroy accepts NULL. Other calls require a live object on its owner thread.
 * Read returns borrowed content until update/destroy; all outputs are required.
 * The host owns font selection, rendering order and process registration. */
MvResult mv_text_create(const char *value, int x, int y, int alignment, MvText **out);
MvResult mv_text_update(MvText *text, const char *value, int x, int y, int alignment);
MvResult mv_text_read(const MvText *text, const char **value, int *x, int *y, int *alignment);
void mv_text_destroy(MvText *text);
#ifdef __cplusplus
}
#endif
#endif
