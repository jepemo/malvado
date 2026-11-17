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
#include "malvado/platform.h"
#include "raylib.h"
#include "rlgl.h"
#include <android/log.h>
#include <stdlib.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <android/asset_manager.h>
#include <stdio.h>
#include <unistd.h>
#include "lifecycle.h"
extern void malvado_rust_entry(void);
void
acceptance_log(const char *message)
{
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s %s", ACCEPTANCE_TOKEN, message);
}
int
acceptance_pixel(void)
{
    mv_window_flush();
    Image pixels = {0};
    pixels.width = GetRenderWidth();
    pixels.height = GetRenderHeight();
    pixels.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    pixels.mipmaps = 1;
    pixels.data = rlReadScreenPixels(pixels.width, pixels.height);
    if (!pixels.data) return 0;
    Color pixel = GetImageColor(pixels, pixels.width / 2, pixels.height / 2);
    UnloadImage(pixels);
    return pixel.r > 240 && pixel.g < 15 && pixel.b < 15;
}
int
acceptance_audio(void)
{
    return ACCEPTANCE_AUDIO;
}
int
acceptance_failure(void)
{
    return ACCEPTANCE_FAILURE;
}
int
acceptance_mode(void)
{
    return ACCEPTANCE_LIFECYCLE;
}
void
acceptance_lifecycle_frame(int verified)
{
    keyboard_frame();
    mv_window_flush();
    lifecycle_frame(verified);
}
void
acceptance_lifecycle_finish(void)
{
    lifecycle_finish();
}
/* Fixed verification payload only. This is not a public APK extractor. */
static void
stage_assets(void)
{
    struct android_app *app = GetAndroidApp();
    assert(app && app->activity);
    assert(chdir(app->activity->internalDataPath) == 0);
    const char *names[] = {"message.bin", "atlas.json", "red.png", "silence.wav", "silence.ogg", "empty.wav"};
    for (unsigned i = 0; i < (ACCEPTANCE_AUDIO ? 6u : 3u); ++i) {
        AAsset *asset = AAssetManager_open(app->activity->assetManager, names[i], AASSET_MODE_BUFFER);
        if (i == 2 && ACCEPTANCE_FAILURE == 2) {
            assert(!asset && access(names[i], F_OK) != 0);
            continue;
        }
        assert(asset);
        off_t size = AAsset_getLength(asset);
        assert(size > 0 && size <= 4 * 1024 * 1024);
        const void *bytes = AAsset_getBuffer(asset);
        assert(bytes);
        FILE *file = fopen(names[i], "wb");
        assert(file);
        assert(fwrite(bytes, 1, (size_t)size, file) == (size_t)size);
        assert(fclose(file) == 0);
        AAsset_close(asset);
    }
}
int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    acceptance_log("Rust entry");
    lifecycle_begin();
    stage_assets();
    malvado_rust_entry();
    if (IsWindowReady() || (ACCEPTANCE_AUDIO && IsAudioDeviceReady())) abort();
    if (ACCEPTANCE_FAILURE == 3) {
        struct android_app *app = GetAndroidApp();
        assert(app && app->destroyRequested && !app->window && !app->pendingWindow);
        assert(!IsAudioDeviceReady());
        acceptance_log("startup cancellation passed");
        return 0;
    }
    lifecycle_cleaned();
    acceptance_log(ACCEPTANCE_FAILURE ? "Rust failure cleanup passed" : "window pixels and shutdown passed");
    return 0;
}
