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
#ifndef MALVADO_PROCESS_H
#define MALVADO_PROCESS_H
#include "malvado/malvado.h"
typedef struct MvProcesses MvProcesses;
typedef enum {
    MV_END_KILLED, MV_END_RETURNED, MV_END_ERROR
} MvEndReason;
/* A dead process is available until cleanup or through the most recent tick. */
int mv_process_end_reason(MvContext *ctx, MvProcessId id, MvEndReason *reason);
MvProcesses *mv_processes_create(MvContext *ctx);
void mv_processes_destroy(MvProcesses *processes);
int mv_processes_can_destroy(const MvProcesses *processes);
MvProcesses *mv_context_processes(MvContext *ctx);
#endif
