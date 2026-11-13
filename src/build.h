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
#ifndef MALVADO_BUILD_H
#define MALVADO_BUILD_H
#include <stddef.h>
/* Project language comes from existing entry files; Cargo owns Rust metadata. */
int malvado_rust_build(int check);
int malvado_rust_run(void);
int malvado_rust_binary(char *path, size_t size);
int malvado_rust_dependency(char *path, size_t size);
int malvado_native_project(void);
int malvado_native_build(void);
int malvado_native_run(void);
#endif
