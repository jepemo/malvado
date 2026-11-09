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

static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void check_frame(void *user)
{
    unsigned *checked = user;
    MalvadoProcessInfo processes[9];
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 9, &total, &tick) && total == 9);
    if (tick < 2) return;
    unsigned stage = tick < 10 ? 0 : tick < 20 ? 1 : tick < 30 ? 2 : 3;
    if (*checked & (1u << stage)) return;
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color red = pixel(image, 7, 12), immediate = pixel(image, 11, 16);
    Color clipped = pixel(image, 16, 12), blue = pixel(image, 48, 68);
    Color blue_clip = pixel(image, 44, 68), inventory = pixel(image, 25, 95);
    Color inventory_clip = pixel(image, 55, 95), hud = pixel(image, 80, 20);
    Color restored = pixel(image, 145, 105);
    if (stage < 3) {
        assert(red.r > 240 && red.g < 10 && red.b < 10);
        assert(immediate.g > 240 && immediate.r < 10);
        assert(clipped.r < 10 && clipped.g < 10 && clipped.b < 10);
        assert(blue_clip.r < 10 && blue_clip.g < 10 && blue_clip.b < 10);
        if (stage == 1) assert(blue.r < 10 && blue.g < 10 && blue.b < 10);
        else assert(blue.b > 240 && blue.r < 10 && blue.g < 10);
        assert(inventory.r > 240 && inventory.g > 240 && inventory.b < 10);
        assert(inventory_clip.r < 10 && inventory_clip.g < 10 && inventory_clip.b < 10);
        assert(hud.g > 240 && hud.r < 10 && hud.b < 10);
        assert(restored.r > 240 && restored.g > 240 && restored.b > 240);
        int text_pixels = 0;
        for (int y = 5; y < 25; ++y) {
            for (int x = 110; x < 125; ++x) {
                Color text = pixel(image, x, y);
                if (text.r > 200 && text.b > 200 && text.g < 20) ++text_pixels;
            }
        }
        assert(text_pixels > 0);
    } else {
        Color background = pixel(image, 155, 115);
        assert(background.r < 10 && background.g > 120 && background.g < 140);
        assert(background.b > 120 && background.b < 140);
        assert(red.r > 110 && red.r < 140 && red.g > 120 && red.b > 120);
        assert(restored.r > 110 && restored.r < 140 && restored.g > 240 && restored.b > 240);
    }
    UnloadImage(image);
    *checked |= 1u << stage;
    if (stage == 3) {
        assert(*checked == 15);
        for (size_t i = 0; i < total; ++i)
            assert(malvado_debug_signal(processes[i].id, "kill"));
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    unsigned checked = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &checked);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && checked == 15);
    return 0;
}
