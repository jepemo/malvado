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

#ifndef MALVADO_INPUT_INTERNAL_H
#define MALVADO_INPUT_INTERNAL_H
#include "malvado/platform.h"
/* Backend-owned window lifetime and per-render event capture. */
void mv_keyboard_open(void);
void mv_keyboard_reset(void);
void mv_keyboard_sample(MvKey key, int pressed, int released);
/* Derive observed edges when a backend cannot supply reliable edge state. */
void mv_keyboard_sample_held(MvKey key, int down);
/* Append an observed scalar to the backend text queue; ignore unavailable
 * input, NUL, invalid scalars and a full queue. Headless sampling is neutral. */
void mv_text_input_sample(uint32_t codepoint);
#ifdef __ANDROID__
/* Called by the current Android editor on the Android UI thread. */
void mv_text_input_commit_utf16(const uint16_t *text, size_t length);
#endif
#endif
