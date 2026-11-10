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
    ++*calls;
    return MV_STEP_DONE;
}

int
main(void)
{
    mv_keyboard_reset();
    mv_keyboard_sample(MV_KEY_SPACE, 1, 1);
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_SPACE) && !mv_key_released(MV_KEY_SPACE));
    mv_keyboard_open();
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    assert(!mv_key_pressed(MV_KEY_SPACE));
    /* Several rendered frames without a tick must retain the event. */
    for (int i = 0; i < 4; ++i)
        mv_keyboard_sample(MV_KEY_SPACE, 0, 0);
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
    }
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
    mv_keyboard_open();
    mv_input_tick();
    assert(!mv_key_pressed(MV_KEY_A) && !mv_key_released(MV_KEY_SPACE));
    mv_keyboard_sample(MV_KEY_SPACE, 1, 0);
    mv_input_tick();
    mv_keyboard_reset();
    assert(!mv_key_pressed(MV_KEY_SPACE));
    mv_context_destroy(ctx);
    return 0;
}
