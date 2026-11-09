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
#include <string.h>
#include "raylib.h"
#include "runner/runtime.h"

typedef struct { int follow, checked; } Capture;
static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}
static void check_frame(void *user)
{
    Capture *capture = user;
    if (capture->checked) return;
    MalvadoProcessInfo processes[3];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 3, &count, &tick));
    if (tick < 8) return;
    assert(count == 3);
    Image image = LoadImageFromScreen();
    assert(image.data);
    if (capture->follow) {
        Color player = pixel(image, 320, 240);
        Color wall = pixel(image, 500 - (int)tick * 3, 240);
        Color old = pixel(image, 500, 240);
        assert(player.b > 240 && player.r > 70 && player.r < 90 && player.g > 180);
        assert(wall.r > 240 && wall.g > 150 && wall.g < 180 && wall.b < 90);
        assert(old.r == 25 && old.g == 30 && old.b == 45);
    } else {
        Color red = pixel(image, 100, 200), green = pixel(image, 200, 200);
        Color blue = pixel(image, 300, 200), clipped = pixel(image, 75, 200);
        Color world = pixel(image, 600, 400);
        assert(red.r > 220 && red.g < 90 && red.b < 90);
        assert(green.g > 200 && green.r < 90);
        assert(blue.b > 230 && blue.r < 90);
        assert(clipped.r == 25 && clipped.g == 30 && clipped.b == 45);
        assert(world.r > 170 && world.g < 90 && world.b > 210);
    }
    int letters = 0;
    for (int y = 20; y < 45; ++y)
        for (int x = 20; x < 500; ++x) {
            Color text = pixel(image, x, y);
            if (text.r > 220 && text.g > 220 && text.b > 220) ++letters;
        }
    assert(letters > 20);
    UnloadImage(image);
    capture->checked = 1;
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    Capture capture = {.follow = !strcmp(argv[2], "follow")};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &capture);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && capture.checked);
    return 0;
}
