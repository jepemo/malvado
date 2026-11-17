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

#ifndef MALVADO_ANDROID_WINDOW_H
#define MALVADO_ANDROID_WINDOW_H

#include "malvado/platform.h"

int mv_android_key_down(MvKey key);
void mv_android_window_attach(void);
void mv_android_window_detach(void);
int mv_android_window_close_requested(void);

#endif
