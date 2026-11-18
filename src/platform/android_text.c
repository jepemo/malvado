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

uint32_t
mv_android_key_codepoint(struct android_app *app, int device, int key, int meta)
{
    if (!app || !app->activity || !app->activity->vm) return 0;
    JavaVM *vm = app->activity->vm;
    JNIEnv *env = NULL;
    int attached = 0;
    jint status = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) return 0;
        attached = 1;
    } else if (status != JNI_OK) return 0;
    uint32_t codepoint = 0;
    /* Preserve an exception owned by another JNI caller. */
    if (!(*env)->ExceptionCheck(env)) {
        jclass type = (*env)->FindClass(env, "android/view/KeyCharacterMap");
        jobject map = NULL;
        if (type) {
            jmethodID load = (*env)->GetStaticMethodID(env, type, "load", "(I)Landroid/view/KeyCharacterMap;");
            if (load) map = (*env)->CallStaticObjectMethod(env, type, load, device);
            if (map && !(*env)->ExceptionCheck(env)) {
                jmethodID get = (*env)->GetMethodID(env, type, "get", "(II)I");
                if (get) codepoint = (uint32_t)(*env)->CallIntMethod(env, map, get, key, meta);
            }
            if (map) (*env)->DeleteLocalRef(env, map);
            (*env)->DeleteLocalRef(env, type);
        }
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
            codepoint = 0;
        }
    }
    if (attached) (*vm)->DetachCurrentThread(vm);
    /* Android marks dead keys with the high bit. Do not emit an accent as a
     * completed character or pretend that IME composition is implemented. */
    if (codepoint < 32 || (codepoint >= 127 && codepoint <= 159) || codepoint > 0x10ffff ||
        (codepoint >= 0xd800 && codepoint <= 0xdfff)) return 0;
    return codepoint;
}
