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
int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    acceptance_log("Rust entry");
    malvado_rust_entry();
    if (IsWindowReady()) abort();
    acceptance_log("window pixels and shutdown passed");
    return 0;
}
