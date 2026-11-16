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

#include "android_window.h"
#include <android_native_app_glue.h>

extern struct android_app *GetAndroidApp(void);
static struct android_app *window_app;
static int32_t (*previous_input)(struct android_app *, AInputEvent *);
static int close_requested;

static int32_t
window_input(struct android_app *app, AInputEvent *event)
{
    int32_t handled = previous_input ? previous_input(app, event) : 0;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
        AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            close_requested = 1;
        return 1;
    }
    return handled;
}

void
mv_android_window_attach(void)
{
    window_app = GetAndroidApp();
    close_requested = 0;
    if (!window_app) return;
    previous_input = window_app->onInputEvent;
    window_app->onInputEvent = window_input;
}

void
mv_android_window_detach(void)
{
    if (window_app && window_app->onInputEvent == window_input)
        window_app->onInputEvent = previous_input;
    window_app = NULL;
    previous_input = NULL;
    close_requested = 0;
}

int
mv_android_window_close_requested(void)
{
    return close_requested;
}
