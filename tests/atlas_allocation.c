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
#include "cJSON.h"
#include "malvado/atlas.h"

static size_t calls, fail_at, outstanding;

void *test_atlas_malloc(size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = malloc(size);
    if (result) ++outstanding;
    return result;
}

void *test_atlas_calloc(size_t count, size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = calloc(count, size);
    if (result) ++outstanding;
    return result;
}

void test_atlas_free(void *pointer)
{
    if (pointer) {
        assert(outstanding > 0);
        --outstanding;
    }
    free(pointer);
}

static void
check_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file);
    char buffer[2048];
    size_t length = fread(buffer, 1, sizeof(buffer), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    cJSON_Hooks hooks = {test_atlas_malloc, test_atlas_free};
    cJSON_InitHooks(&hooks);
    size_t failures = 0;
    for (size_t index = 1; index < 1000; ++index) {
        calls = 0;
        fail_at = index;
        const char *error = NULL;
        MvAtlas *atlas = mv_atlas_parse_aseprite(buffer, length, 8, 8, &error);
        if (atlas) {
            assert(!error && index > calls);
            mv_atlas_destroy(atlas);
            assert(outstanding == 0);
            break;
        }
        assert(error && outstanding == 0);
        ++failures;
    }
    assert(failures > 0 && failures < 999);
    cJSON_InitHooks(NULL);
}

int
main(int argc, char **argv)
{
    assert(argc == 3);
    check_file(argv[1]);
    check_file(argv[2]);
    return 0;
}
