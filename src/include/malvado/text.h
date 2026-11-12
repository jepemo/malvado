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
/* Context-local positive IDs, never reused in that context (maximum INT64_MAX).
 * Owner NULL means host text; otherwise it must be a live context-bound process.
 * The context owns copied content until remove, owner cleanup or shutdown.
 * Sleep/freeze retain entries; kill/return release at the scheduler safe point,
 * after process destructors. Host entries survive until remove or shutdown.
 * Missing/zero IDs return MV_NOT_FOUND. Invalid arguments return
 * MV_INVALID_ARGUMENT; create during shutdown returns MV_BUSY. Creation and
 * update are transactional. No font or platform resource is retained.
 * Read borrows content until update/remove/cleanup/shutdown. All outputs required.
 * IDs snapshot is in creation order. required is always written on valid calls;
 * NULL ids with capacity 0 queries count. A short buffer returns MV_OVERFLOW
 * without writing any IDs. All calls require owner-thread context access. */
MvResult mv_text_entry_create(MvContext *ctx, const MvProcessRef *owner,
    const char *value, int x, int y, int alignment, uint64_t *out);
MvResult mv_text_entry_update(MvContext *ctx, uint64_t id,
    const char *value, int x, int y, int alignment);
MvResult mv_text_entry_read(MvContext *ctx, uint64_t id,
    const char **value, int *x, int *y, int *alignment);
MvResult mv_text_entry_remove(MvContext *ctx, uint64_t id);
MvResult mv_text_entries(MvContext *ctx, uint64_t *ids, size_t capacity, size_t *required);
#ifdef __cplusplus
}
#endif
#endif
