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
#include "malvado/fpg.h"

static size_t calls, fail_at, outstanding;

void *test_manifest_malloc(size_t size)
{
    if (++calls == fail_at) return NULL;
    void *value = malloc(size);
    if (value) ++outstanding;
    return value;
}

void *test_manifest_calloc(size_t count, size_t size)
{
    if (++calls == fail_at) return NULL;
    void *value = calloc(count, size);
    if (value) ++outstanding;
    return value;
}

void test_manifest_free(void *value)
{
    if (value) {
        assert(outstanding);
        --outstanding;
    }
    free(value);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    unsigned char bytes[2048];
    size_t length = fread(bytes, 1, sizeof(bytes), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    const char *error = NULL;
    MvFpg *fpg = mv_fpg_parse(bytes, length, &error);
    assert(fpg && !error);
    size_t size = mv_fpg_json_size(fpg, 1);
    char *json = malloc(size);
    assert(json && mv_fpg_copy_json(fpg, 1, json, size));
    cJSON_Hooks hooks = {test_manifest_malloc, test_manifest_free};
    cJSON_InitHooks(&hooks);
    size_t failures = 0;
    for (size_t index = 1; index < 1000; ++index) {
        calls = 0;
        fail_at = index;
        MvFpgManifest *manifest = mv_fpg_manifest_parse(json, size - 1, &error);
        if (manifest) {
            assert(!error && index > calls);
            mv_fpg_manifest_destroy(manifest);
            assert(!outstanding);
            break;
        }
        assert(error && !outstanding);
        ++failures;
    }
    assert(failures > 0 && failures < 999);
    cJSON_InitHooks(NULL);
    free(json);
    mv_fpg_destroy(fpg);
    return 0;
}
