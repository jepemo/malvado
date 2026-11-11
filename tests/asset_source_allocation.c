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
#include "malvado/asset.h"
#include "cJSON.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static size_t calls, fail_at, outstanding;
void *test_source_malloc(size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = malloc(size);
    if (result) ++outstanding;
    return result;
}
void *test_source_calloc(size_t count, size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = calloc(count, size);
    if (result) ++outstanding;
    return result;
}
void test_source_free(void *pointer)
{
    if (pointer) { assert(outstanding); --outstanding; }
    free(pointer);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    cJSON_Hooks hooks = {test_source_malloc, test_source_free};
    cJSON_InitHooks(&hooks);
    size_t rejected = 0, unknown = 0;
    for (size_t index = 1; index < 1000; ++index) {
        calls = 0;
        fail_at = index;
        const char *error = NULL;
        MvAssetSource *source = mv_asset_source_open(argv[1], &error);
        if (!source) { assert(error); ++rejected; }
        else {
            assert(!error);
            if (mv_asset_source_format(source) == MV_ASSET_UNKNOWN) ++unknown;
            else assert(mv_asset_source_format(source) == MV_ASSET_ATLAS);
        }
        mv_asset_source_destroy(source);
        assert(!outstanding);
        if (calls < index) break;
    }
    assert(rejected == 3 && unknown > 0);
    cJSON_InitHooks(NULL);
    return 0;
}
