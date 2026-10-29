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

#ifndef MALVADO_RUNTIME_H
#define MALVADO_RUNTIME_H

#include <stddef.h>
#include <stdint.h>
#include "malvado/malvado.h"

typedef struct {
    int64_t id;
    int64_t parent_id;
    int64_t type_id;
    int64_t priority;
    size_t messages;
    const char *status; /* Static string; not owned by the caller. */
} MalvadoProcessInfo;

typedef void (*MalvadoDebugHook)(void *user);

/* Call on the runner thread only, from the hook between simulation ticks. */
void malvado_debug_set_hook(MalvadoDebugHook hook, void *user);
/* Called on the runner thread after drawing and before presenting a frame. */
void malvado_debug_set_frame_hook(MalvadoDebugHook hook, void *user);
int malvado_debug_snapshot(MalvadoProcessInfo *out, size_t capacity,
                           size_t *total, int64_t *tick);
int malvado_debug_signal(int64_t id, const char *signal);
int malvado_debug_pause(int paused);
int malvado_debug_step(void);
/* Borrowed context, valid only while the runner is active on its owner thread. */
MvContext *malvado_debug_context(void);

int malvado_run(int argc, char **argv);

#endif
