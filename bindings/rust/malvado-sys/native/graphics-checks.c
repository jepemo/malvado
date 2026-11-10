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
/* Pixel assertions used only by the Rust verification executables. */
#include "malvado/platform.h"
#include "raylib.h"
#include <stdlib.h>

int
malvado_test_pixel(int x, int y, int r, int g, int b)
{
    int width = GetScreenWidth(), height = GetScreenHeight();
    if (width <= 0 || height <= 0 || x < 0 || y < 0 || x >= width || y >= height)
        return 0;
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    if (!screen.data)
        return 0;
    Color pixel = GetImageColor(screen, x * screen.width / width, y * screen.height / height);
    UnloadImage(screen);
    return abs((int)pixel.r - r) < 8 && abs((int)pixel.g - g) < 8 && abs((int)pixel.b - b) < 8;
}
