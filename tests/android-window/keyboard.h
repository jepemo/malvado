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

#ifndef MALVADO_TEST_ANDROID_KEYBOARD_H
#define MALVADO_TEST_ANDROID_KEYBOARD_H

static unsigned keyboard_index;
static int keyboard_announced;
static int keyboard_down, keyboard_pressed, keyboard_released, keyboard_complete;

static int
keyboard_mode(void)
{
    return ACCEPTANCE_LIFECYCLE == 4 || ACCEPTANCE_LIFECYCLE == 5 || ACCEPTANCE_LIFECYCLE == 6;
}

static void
keyboard_frame(void)
{
    if (!keyboard_mode() || keyboard_complete) return;
    assert(!mv_key_down(MV_KEY_B) && !mv_key_pressed(MV_KEY_B) && !mv_key_released(MV_KEY_B));
    if (keyboard_index == 2) {
        if (mv_key_down(MV_KEY_A) || mv_key_pressed(MV_KEY_A) || mv_key_released(MV_KEY_A) ||
            mv_key_down(MV_KEY_LEFT) || mv_key_pressed(MV_KEY_LEFT) || mv_key_released(MV_KEY_LEFT)) return;
        keyboard_complete = 1;
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s keyboard passed", ACCEPTANCE_TOKEN);
        return;
    }
    MvKey key = keyboard_index == 0 ? MV_KEY_A : MV_KEY_LEFT;
    int down = mv_key_down(key), pressed = mv_key_pressed(key), released = mv_key_released(key);
    /* Queries share a tick snapshot. Reading an edge must not consume it. */
    assert(mv_key_down(key) == down && mv_key_pressed(key) == pressed && mv_key_released(key) == released);
    keyboard_down |= down;
    keyboard_pressed |= pressed;
    keyboard_released |= released;
    if (keyboard_down && keyboard_pressed && !keyboard_announced) {
        keyboard_announced = 1;
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s keyboard %s held",
                            ACCEPTANCE_TOKEN, keyboard_index == 0 ? "a" : "left");
    }
    if (keyboard_down && keyboard_pressed && keyboard_released && !down) {
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s keyboard %s released",
                            ACCEPTANCE_TOKEN, keyboard_index == 0 ? "a" : "left");
        keyboard_index++;
        keyboard_down = keyboard_pressed = keyboard_released = keyboard_announced = 0;
    }
}
#endif
