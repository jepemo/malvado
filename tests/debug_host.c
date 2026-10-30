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
#include <stdint.h>
#include <string.h>
#include "runner/runtime.h"

typedef struct {
    int phase;
    int64_t id;
} Probe;

static void
inspect(void *user)
{
    Probe *probe = user;
    MalvadoProcessInfo info;
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(&info, 1, &total, &tick));
    if (probe->phase == 0) {
        char *nested[] = {"malvado-run", "--headless", "unused.lua"};
        assert(malvado_run(3, nested) == 2);
        assert(tick == 1 && total == 1 && strcmp(info.status, "running") == 0);
        probe->id = info.id;
        assert(malvado_debug_signal(info.id, "freeze"));
        assert(malvado_debug_pause(1));
        probe->phase = 1;
    } else if (probe->phase == 1) {
        assert(tick == 1 && strcmp(info.status, "frozen") == 0);
        assert(malvado_debug_step());
        probe->phase = 2;
    } else if (probe->phase == 2) {
        assert(tick == 2 && strcmp(info.status, "frozen") == 0);
        assert(malvado_debug_signal(probe->id, "wakeup"));
        assert(malvado_debug_pause(0));
        probe->phase = 3;
    } else if (probe->phase == 3) {
        assert(tick == 3 && strcmp(info.status, "running") == 0);
        assert(malvado_debug_signal(probe->id, "kill"));
        probe->phase = 4;
    } else {
        assert(tick == 4 && total == 0);
        probe->phase = 5;
    }
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    Probe probe = {0};
    char *args[] = {"malvado-run", "--headless", argv[1]};
    size_t total;
    int64_t tick;
    assert(!malvado_debug_snapshot(NULL, 0, &total, &tick));
    malvado_debug_set_hook(inspect, &probe);
    int status = malvado_run(3, args);
    malvado_debug_set_hook(NULL, NULL);
    assert(status == 0 && probe.phase == 5);
    assert(!malvado_debug_snapshot(NULL, 0, &total, &tick));
    Probe second = {0};
    malvado_debug_set_hook(inspect, &second);
    status = malvado_run(3, args);
    malvado_debug_set_hook(NULL, NULL);
    assert(status == 0 && second.phase == 5);
    assert(!malvado_debug_snapshot(NULL, 0, &total, &tick));
    return 0;
}
