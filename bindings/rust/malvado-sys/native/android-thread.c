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
#include <android_native_app_glue.h>
#include <pthread.h>
extern struct android_app *GetAndroidApp(void);
int
mv_rust_android_app_thread(void)
{
    struct android_app *app = GetAndroidApp();
    return app && pthread_equal(pthread_self(), app->thread);
}
