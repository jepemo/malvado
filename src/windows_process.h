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
#ifndef MALVADO_WINDOWS_PROCESS_H
#define MALVADO_WINDOWS_PROCESS_H
#include <stddef.h>
/* Synchronous CLI child. SDK environment overrides are child-local. Optional
 * stdout capture is bounded and NUL-terminated; overflow returns failure after
 * draining the pipe. All process/pipe/environment storage is released. */
int malvado_windows_process(char *const args[], const char *sdk, char *output, size_t size);
#endif
