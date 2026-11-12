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

#ifndef MALVADO_RENDER_INTERNAL_H
#define MALVADO_RENDER_INTERNAL_H
#include "malvado/malvado.h"
/* Binding-only preparation before native graph/command collection. Return 0
 * to suppress only the graph, nonzero to include it. Commands remain visible.
 * IDs/order are snapshotted; native values are read after preparation. The host
 * may synchronize descriptors/resources, but must not tick, draw, enter scopes,
 * destroy the context or close services. Queued drawing is flushed before each
 * callback so binding allocation/GC can safely release prior image references.
 * No callback/data is retained. NULL prepares every graph unchanged. */
typedef int (*MvRenderPrepare)(MvProcessRef process, void *user);
int malvado_window_draw_process_graphics(MvContext *ctx, MvRenderPrepare prepare, void *user);
#endif
