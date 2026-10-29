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
MvProcesses *mv_processes_create(MvContext *ctx);
void mv_processes_destroy(MvProcesses *processes);
int mv_processes_can_destroy(const MvProcesses *processes);
MvProcesses *mv_context_processes(MvContext *ctx);
#endif
