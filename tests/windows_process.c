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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "windows_process.h"

int
main(int argc, char **argv)
{
    if (argc > 1) {
        assert(argc == 6 && !strcmp(argv[1], "--child"));
        assert(!strcmp(argv[2], ""));
        assert(!strcmp(argv[3], "two words"));
        assert(!strcmp(argv[4], "q\"uote\\"));
        assert(!strcmp(argv[5], "&no-shell$()"));
        assert(!strcmp(getenv("RUSTUP_AUTO_INSTALL"), "0"));
        assert(!strcmp(getenv("MALVADO_SDK_PREFIX"), "sdk path with spaces"));
        puts("child-ok");
        return 7;
    }
    const char *previous = getenv("RUSTUP_AUTO_INSTALL");
    char *saved = previous ? _strdup(previous) : NULL;
    char *args[] = {argv[0], "--child", "", "two words", "q\"uote\\", "&no-shell$()", NULL};
    char output[128];
    assert(malvado_windows_process(args, "sdk path with spaces", output, sizeof(output)) == 7);
    assert(strstr(output, "child-ok"));
    previous = getenv("RUSTUP_AUTO_INSTALL");
    assert(saved ? previous && !strcmp(previous, saved) : !previous);
    free(saved);
    assert(malvado_windows_process(args, "sdk path with spaces", output, 2) != 0);
    char *missing[] = {"malvado-missing-tool-781936.exe", NULL};
    assert(malvado_windows_process(missing, NULL, output, sizeof(output)) != 0);
    assert(output[0] == 0);
    return 0;
}
