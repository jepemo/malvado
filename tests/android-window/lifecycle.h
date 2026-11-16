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

#ifndef MALVADO_TEST_ANDROID_LIFECYCLE_H
#define MALVADO_TEST_ANDROID_LIFECYCLE_H
#include <android_native_app_glue.h>
#include <stdlib.h>
extern struct android_app *GetAndroidApp(void);
static struct android_app *lifecycle_app;
static void (*lifecycle_previous)(struct android_app *, int32_t);
static int lifecycle_pauses, lifecycle_resumes, lifecycle_pending, lifecycle_ready;
static int touch_down, touch_released, touch_first_x, touch_max_x;
#include "keyboard.h"

static void
lifecycle_command(struct android_app *app, int32_t command)
{
    if (lifecycle_previous) lifecycle_previous(app, command);
    if (command == APP_CMD_PAUSE) lifecycle_pauses++;
    if (command == APP_CMD_RESUME && lifecycle_pauses) {
        lifecycle_resumes++;
        lifecycle_pending = 1;
    }
}

static void
lifecycle_frame(int visible)
{
    if (!ACCEPTANCE_LIFECYCLE) return;
    keyboard_frame();
    if (!lifecycle_app) {
        lifecycle_app = GetAndroidApp();
        assert(lifecycle_app);
        lifecycle_previous = lifecycle_app->onAppCmd;
        lifecycle_app->onAppCmd = lifecycle_command;
    }
    if (visible && !lifecycle_ready) {
        lifecycle_ready = 1;
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lifecycle ready", ACCEPTANCE_TOKEN);
    }
    if (ACCEPTANCE_LIFECYCLE == 3) {
        int x, y;
        if (mv_mouse_down(MV_MOUSE_LEFT)) {
            assert(mv_mouse_position(&x, &y));
            assert(!mv_mouse_down(MV_MOUSE_MIDDLE) && !mv_mouse_down(MV_MOUSE_RIGHT));
            if (!touch_down) {
                assert(abs(x - 160) <= 16 && abs(y - 120) <= 16);
                touch_first_x = touch_max_x = x;
                touch_down = 1;
            }
            if (x > touch_max_x) touch_max_x = x;
        } else if (touch_down && !touch_released) {
            assert(touch_max_x > touch_first_x + 4);
            touch_released = 1;
            __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s touch released", ACCEPTANCE_TOKEN);
        }
    }
    if (lifecycle_pending) {
        Image image = {0};
        image.width = GetRenderWidth();
        image.height = GetRenderHeight();
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        image.mipmaps = 1;
        image.data = rlReadScreenPixels(image.width, image.height);
        assert(image.data);
        Color pixel = GetImageColor(image, image.width / 2, image.height / 2);
        int red = pixel.r > 240 && pixel.g < 15 && pixel.b < 15;
        UnloadImage(image);
        if (red) {
            lifecycle_pending = 0;
            __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lifecycle resumed", ACCEPTANCE_TOKEN);
        }
    }
}

static void
lifecycle_finish(void)
{
    if (!ACCEPTANCE_LIFECYCLE) return;
    assert(lifecycle_ready);
    if (ACCEPTANCE_LIFECYCLE == 3) assert(touch_down && touch_released);
    if (keyboard_mode()) assert(keyboard_complete);
    if (ACCEPTANCE_LIFECYCLE == 2 || ACCEPTANCE_LIFECYCLE == 5)
        assert(lifecycle_pauses && lifecycle_resumes && !lifecycle_pending);
    assert(lifecycle_app && lifecycle_app->onAppCmd == lifecycle_command);
    lifecycle_app->onAppCmd = lifecycle_previous;
}
#endif
