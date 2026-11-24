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
#include "malvado/malvado.h"
#include "malvado/platform.h"
#include "input_internal.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

static MvStepResult
observe(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    unsigned *calls = user;
    assert(mv_key_pressed(MV_KEY_SPACE));
    assert(mv_key_pressed(MV_KEY_SPACE));
    assert(!mv_key_released(MV_KEY_SPACE));
    assert(mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_pressed(MV_MOUSE_LEFT));
    assert(!mv_mouse_released(MV_MOUSE_LEFT));
    assert(mv_pad_button_pressed(3, MV_PAD_A) && mv_pad_button_pressed(3, MV_PAD_A));
    assert(!mv_pad_button_pressed(0, MV_PAD_A));
    ++*calls;
    return MV_STEP_DONE;
}

int
main(void)
{
    mv_input_reset();
    mv_keyboard_sample(MV_KEY_SPACE, 1, 1);
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_SPACE) && !mv_key_released(MV_KEY_SPACE));
    mv_input_open();
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    mv_mouse_sample(MV_MOUSE_LEFT, 1, 0);
    mv_pad_sample(0, 1, 1u << MV_PAD_A);
    mv_pad_sample(3, 1, 0);
    mv_pad_sample(3, 1, 1u << MV_PAD_A);
    assert(!mv_key_pressed(MV_KEY_SPACE));
    assert(!mv_mouse_pressed(MV_MOUSE_LEFT) && !mv_pad_button_pressed(3, MV_PAD_A));
    /* Several rendered frames without a tick must retain the event. */
    for (int i = 0; i < 4; ++i) {
        mv_keyboard_sample(MV_KEY_SPACE, 0, 0);
        mv_mouse_sample(MV_MOUSE_LEFT, 0, 0);
        mv_pad_sample(3, 1, 1u << MV_PAD_A);
    }
    mv_input_tick();
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    unsigned calls = 0;
    MvProcessId id;
    assert(mv_process_spawn(ctx, 0, 0, observe, NULL, &calls, &id) == MV_OK);
    assert(mv_process_spawn(ctx, 0, 0, observe, NULL, &calls, &id) == MV_OK);
    assert(mv_process_tick(ctx) == MV_OK && calls == 2);
    /* Catch-up ticks must not repeat an event without another capture. */
    for (int i = 0; i < 5; ++i) {
        mv_input_tick();
        assert(!mv_key_pressed(MV_KEY_SPACE) && !mv_key_released(MV_KEY_SPACE));
        assert(!mv_mouse_pressed(MV_MOUSE_LEFT) && !mv_pad_button_pressed(3, MV_PAD_A));
    }
    mv_mouse_sample(MV_MOUSE_MIDDLE, 1, 0);
    mv_mouse_sample(MV_MOUSE_MIDDLE, 0, 1);
    mv_pad_sample(3, 1, 1u << MV_PAD_B);
    mv_pad_sample(3, 1, 0);
    mv_input_tick();
    assert(mv_mouse_pressed(MV_MOUSE_MIDDLE) && mv_mouse_released(MV_MOUSE_MIDDLE));
    assert(mv_pad_button_pressed(3, MV_PAD_B) && mv_pad_button_released(3, MV_PAD_B));
    assert(mv_pad_button_released(3, MV_PAD_A) && !mv_pad_button_released(0, MV_PAD_A));
    mv_pad_sample(3, 1, 1u << MV_PAD_A);
    mv_pad_sample(3, 0, 0);
    assert(!mv_pad_button_released(3, MV_PAD_A) && !mv_pad_button_pressed(3, MV_PAD_B));
    mv_pad_sample(3, 1, 1u << MV_PAD_A);
    mv_input_tick();
    assert(!mv_pad_button_pressed(3, MV_PAD_A) && !mv_pad_button_released(3, MV_PAD_A));
    mv_pad_sample(3, 1, 0);
    mv_input_tick();
    assert(mv_pad_button_released(3, MV_PAD_A));
    assert(!mv_pad_button_pressed(-1, MV_PAD_A) && !mv_pad_button_released(4, MV_PAD_A));
    assert(!mv_pad_button_released(3, (MvPadButton)999));
    assert(!mv_mouse_pressed((MvMouseButton)-1) && !mv_mouse_released((MvMouseButton)999));
    mv_keyboard_sample(MV_KEY_SPACE, 0, 1);
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_SPACE) && mv_key_released(MV_KEY_SPACE));
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    mv_keyboard_sample(MV_KEY_SPACE, 0, 1);
    mv_input_tick();
    assert(mv_key_pressed(MV_KEY_SPACE) && mv_key_released(MV_KEY_SPACE));
    assert(!mv_key_pressed(MV_KEY_A));
    assert(!mv_key_pressed((MvKey)-1) && !mv_key_released((MvKey)999));
    mv_keyboard_sample((MvKey)-1, 1, 1);
    mv_keyboard_sample((MvKey)999, 1, 1);
    mv_keyboard_sample(MV_KEY_A, 1, 0);
    mv_mouse_sample(MV_MOUSE_LEFT, 1, 0);
    mv_pad_sample(3, 1, 1u << MV_PAD_A);
    mv_input_open();
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_A) && !mv_key_released(MV_KEY_SPACE));
    assert(!mv_mouse_pressed(MV_MOUSE_LEFT) && !mv_pad_button_pressed(3, MV_PAD_A));
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    mv_input_tick();
    mv_input_reset();
    assert(!mv_key_pressed(MV_KEY_SPACE));
    /* Android must not depend on backend edge history for high key codes. */
    mv_keyboard_sample_held(MV_KEY_LEFT, 1);
    mv_input_open();
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_LEFT));
    mv_keyboard_sample_held((MvKey)-1, 1);
    mv_keyboard_sample_held((MvKey)999, 1);
    MvKey keys[] = {MV_KEY_A, MV_KEY_LEFT, MV_KEY_UP, MV_KEY_TAB, MV_KEY_BACKSPACE};
    for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        MvKey key = keys[i];
        mv_keyboard_sample_held(key, 7);
        mv_input_tick();
        assert(mv_key_pressed(key) && !mv_key_released(key));
        mv_keyboard_sample_held(key, 1);
        mv_input_tick();
        assert(!mv_key_pressed(key) && !mv_key_released(key));
        mv_keyboard_sample_held(key, 0);
        mv_keyboard_sample_held(key, 0);
        mv_input_tick();
        assert(!mv_key_pressed(key) && mv_key_released(key));
        mv_keyboard_sample_held(key, 1);
        mv_keyboard_sample_held(key, 0);
        mv_input_tick();
        assert(mv_key_pressed(key) && mv_key_released(key));
    }
    mv_keyboard_sample_held(MV_KEY_LEFT, 1);
    mv_input_open();
    mv_keyboard_sample_held(MV_KEY_LEFT, 0);
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_LEFT) && !mv_key_released(MV_KEY_LEFT));
    mv_context_destroy(ctx);
    return 0;
}
