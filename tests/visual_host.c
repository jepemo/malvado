/*
 * Malvado - game development with Lua and processes.
 * Project created by Jeremías Pérez; Malvado contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */
#include <assert.h>
#include <stdint.h>
#include "raylib.h"
#include "runner/runtime.h"

typedef struct {
    int phase;
    int64_t worker;
    int64_t controller;
} Probe;

static int
has_light_pixels(void)
{
    Image image = LoadImageFromScreen();
    assert(image.data);
    int light = 0;
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            Color color = GetImageColor(image, x, y);
            if (color.r > 100 || color.g > 100 || color.b > 100)
                light++;
        }
    }
    UnloadImage(image);
    return light != 0;
}

static void
inspect(void *user)
{
    Probe *p = user;
    MalvadoProcessInfo info[2];
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(info, 2, &total, &tick));
    if (p->phase == 0) {
        assert(tick >= 1 && total == 2);
        p->worker = info[0].id;
        p->controller = info[1].id;
        assert(malvado_debug_signal(p->worker, "freeze"));
    } else if (p->phase == 1) {
        assert(has_light_pixels());
        assert(malvado_debug_signal(p->worker, "sleep"));
    } else if (p->phase == 2) {
        assert(!has_light_pixels());
        assert(malvado_debug_signal(p->worker, "wakeup"));
    } else if (p->phase == 3) {
        /* The worker draws during this tick; it is presented on the next frame. */
    } else if (p->phase == 4) {
        assert(has_light_pixels());
        assert(malvado_debug_signal(p->worker, "kill"));
        assert(malvado_debug_signal(p->controller, "kill"));
    }
    p->phase++;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    Probe probe = {0};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(inspect, &probe);
    int status = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(status == 0 && probe.phase >= 5);
    return 0;
}
