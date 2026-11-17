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

int
mv_android_activity_has_focus(struct android_app *app)
{
    if (!app || !app->activity || !app->activity->vm || !app->activity->clazz) return 0;
    JavaVM *vm = app->activity->vm;
    JNIEnv *env = NULL;
    int attached = 0;
    jint status = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) return 0;
        attached = 1;
    } else if (status != JNI_OK) return 0;
    int focused = 0;
    /* Do not consume an exception that belongs to another JNI caller. */
    if (!(*env)->ExceptionCheck(env)) {
        jclass activity = (*env)->GetObjectClass(env, app->activity->clazz);
        if (activity) {
            jmethodID method = (*env)->GetMethodID(env, activity, "hasWindowFocus", "()Z");
            if (method) focused = (*env)->CallBooleanMethod(env, app->activity->clazz, method) == JNI_TRUE;
            (*env)->DeleteLocalRef(env, activity);
        }
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
            focused = 0;
        }
    }
    if (attached) (*vm)->DetachCurrentThread(vm);
    return focused;
}
