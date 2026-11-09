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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include "raylib.h"
#include "runner/runtime.h"

static Color
pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void
check_frame(void *user)
{
    int *checked = user;
    MalvadoProcessInfo processes[7];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 7, &count, &tick));
    if (tick < 2 || *checked) return;
    assert(count == 7);
    Image screen = LoadImageFromScreen();
    Image sheet = LoadImage("examples/aseprite/sheet.png");
    assert(screen.data && sheet.data);
    Color png = pixel(screen, 11, 11);
    Color atlas = pixel(screen, 40, 20);
    Color aseprite = pixel(screen, 60, 20);
    Color expected = GetImageColor(sheet, 2, 2);
    Color original = pixel(screen, 80, 20);
    Color converted = pixel(screen, 100, 20);
    Color transparent = pixel(screen, 79, 20);
    Color converted_transparent = pixel(screen, 99, 20);
    assert(png.r > 240 && png.g < 10);
    assert(atlas.g > 240 && atlas.r < 10);
    assert(aseprite.r == expected.r && aseprite.g == expected.g && aseprite.b == expected.b);
    assert(original.r > 240 && original.g < 10);
    assert(original.r == converted.r && original.g == converted.g && original.b == converted.b);
    assert(transparent.r == 25 && transparent.g == 30 && transparent.b == 45);
    assert(converted_transparent.r == 25 && converted_transparent.g == 30);
    Color shared = pixel(screen, 60, 50);
    assert(shared.r == aseprite.r && shared.g == aseprite.g && shared.b == aseprite.b);
    Color opaque = pixel(screen, 119, 20);
    assert(opaque.r != transparent.r || opaque.g != transparent.g || opaque.b != transparent.b);
    UnloadImage(sheet);
    UnloadImage(screen);
    *checked = 1;
    for (size_t i = 0; i < count; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int checked = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &checked);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && checked);
    return 0;
}
