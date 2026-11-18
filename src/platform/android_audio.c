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
#include "raylib.h"
#include <android_native_app_glue.h>

extern struct android_app *GetAndroidApp(void);

/* Serialize callbacks with device attachment and detachment. The mixer
 * reads the master gain through miniaudio's atomic volume accessor. */
static pthread_mutex_t focus_mutex = PTHREAD_MUTEX_INITIALIZER;
static jlong focus_token;
static jlong next_token;
static float output_gain;

static void JNICALL
focus_changed(JNIEnv *env, jclass activity, jlong token, jboolean focused)
{
    (void)env;
    (void)activity;
    pthread_mutex_lock(&focus_mutex);
    if (token && token == focus_token)
        SetMasterVolume(focused == JNI_TRUE ? output_gain : 0.0f);
    pthread_mutex_unlock(&focus_mutex);
}

static int
set_activity_device(jlong token)
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
            jmethodID method = (*env)->GetMethodID(env, type, "malvadoSetAudioDevice", "(J)V");
            const JNINativeMethod callback = {
                "audioFocusChanged", "(JZ)V", (void *)focus_changed
            };
            if (method && (!token || (*env)->RegisterNatives(env, type, &callback, 1) == JNI_OK)) {
                if (token) {
                    pthread_mutex_lock(&focus_mutex);
                    output_gain = GetMasterVolume();
                    SetMasterVolume(0.0f);
                    focus_token = token;
                    pthread_mutex_unlock(&focus_mutex);
                }
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

void
mv_android_audio_attach(void)
{
    if (next_token == INT64_MAX) next_token = 0;
    if (set_activity_device(++next_token)) return;

    /* A regular NativeActivity has no focus bridge. The audio device keeps
     * its default behavior for SDK consumers that supply their own host. */
    pthread_mutex_lock(&focus_mutex);
    if (focus_token) {
        focus_token = 0;
        SetMasterVolume(output_gain);
    }
    pthread_mutex_unlock(&focus_mutex);
}

void
mv_android_audio_detach(void)
{
    pthread_mutex_lock(&focus_mutex);
    jlong token = focus_token;
    focus_token = 0;
    pthread_mutex_unlock(&focus_mutex);
    if (token) (void)set_activity_device(0);
}
