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
#ifdef __linux__
#define _GNU_SOURCE
#endif

/* Pixel assertions used only by the Rust verification executables. */
#include "malvado/platform.h"
#include "raylib.h"
#include <stdlib.h>
#ifdef __linux__
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static void
pin_test_providers(void)
{
    static int pinned;
    if (pinned || !getenv("MALVADO_PIN_TEST_PROVIDERS"))
        return;
    pinned = 1;
    /* Keep provider code and global roots mapped for LSan's exit-time scan.
     * This is a test fixture; no game calls it or changes driver lifetime. */
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps)
        abort();
    char line[4096];
    while (fgets(line, sizeof(line), maps)) {
        char *path = strchr(line, '/');
        if (!path || (!strstr(path, "/dri/") && !strstr(path, "/libgallium") &&
                      !strstr(path, "/libLLVM") && !strstr(path, "/libGLX_mesa")))
            continue;
        path[strcspn(path, "\n")] = '\0';
        if (!dlopen(path, RTLD_LAZY | RTLD_NODELETE)) {
            fprintf(stderr, "cannot pin graphics provider %s: %s\n", path, dlerror());
            abort();
        }
    }
    fclose(maps);
}
#endif

int
malvado_test_pixel(int x, int y, int r, int g, int b)
{
    int width = GetScreenWidth(), height = GetScreenHeight();
    if (width <= 0 || height <= 0 || x < 0 || y < 0 || x >= width || y >= height)
        return 0;
    mv_window_flush();
#ifdef __linux__
    pin_test_providers();
#endif
    Image screen = LoadImageFromScreen();
    if (!screen.data)
        return 0;
    Color pixel = GetImageColor(screen, x * screen.width / width, y * screen.height / height);
    UnloadImage(screen);
    return abs((int)pixel.r - r) < 8 && abs((int)pixel.g - g) < 8 && abs((int)pixel.b - b) < 8;
}
