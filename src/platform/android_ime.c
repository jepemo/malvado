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
#include "input_internal.h"
#include <android_native_app_glue.h>

extern struct android_app *GetAndroidApp(void);

static pthread_mutex_t editor_mutex = PTHREAD_MUTEX_INITIALIZER;
static jlong editor_token;
static jlong next_token;

static void JNICALL
commit_text(JNIEnv *env, jclass activity, jlong token, jstring text)
{
    (void)activity;
    if (!token || !text) return;
    pthread_mutex_lock(&editor_mutex);
    if (token == editor_token) {
        const jchar *characters = (*env)->GetStringChars(env, text, NULL);
        if (characters) {
            jsize length = (*env)->GetStringLength(env, text);
            mv_text_input_commit_utf16(characters, (size_t)length);
            (*env)->ReleaseStringChars(env, text, characters);
        }
    }
    pthread_mutex_unlock(&editor_mutex);
}

static int
request_keyboard(jlong token)
{
    struct android_app *app = GetAndroidApp();
    if (!app || !app->activity || !app->activity->vm) return 0;
    ANativeActivity *activity = app->activity;
    JavaVM *vm = activity->vm;
    JNIEnv *env = NULL;
    int attached = 0;
    jint status = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) return 0;
        attached = 1;
    } else if (status != JNI_OK) return 0;

    int sent = 0;
    if (!(*env)->ExceptionCheck(env)) {
        jclass type = (*env)->GetObjectClass(env, activity->clazz);
        if (type) {
            jmethodID method = (*env)->GetMethodID(env, type, "malvadoSetKeyboard", "(J)V");
            const JNINativeMethod callback = {
                "keyboardTextCommitted", "(JLjava/lang/String;)V", (void *)commit_text
            };
            if (method && (!token || (*env)->RegisterNatives(env, type, &callback, 1) == JNI_OK)) {
                (*env)->CallVoidMethod(env, activity->clazz, method, token);
                sent = !(*env)->ExceptionCheck(env);
            }
            (*env)->DeleteLocalRef(env, type);
        }
        if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    }
    if (attached) (*vm)->DetachCurrentThread(vm);
    return sent;
}

int
mv_android_keyboard_request(int visible)
{
    pthread_mutex_lock(&editor_mutex);
    if (!visible) {
        editor_token = 0;
    } else if (!editor_token) {
        if (next_token == INT64_MAX) next_token = 0;
        editor_token = ++next_token;
    }
    jlong token = editor_token;
    pthread_mutex_unlock(&editor_mutex);

    int sent = request_keyboard(token);
    if (!sent && token) {
        pthread_mutex_lock(&editor_mutex);
        editor_token = 0;
        pthread_mutex_unlock(&editor_mutex);
    }
    return sent;
}
