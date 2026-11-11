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
void *test_metadata_malloc(size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = malloc(size);
    if (result) ++outstanding;
    return result;
}
void *test_metadata_calloc(size_t count, size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = calloc(count, size);
    if (result) ++outstanding;
    return result;
}
void test_metadata_free(void *pointer)
{
    if (pointer) { assert(outstanding); --outstanding; }
    free(pointer);
}
static void check_source(MvAssetSource *source)
{
    assert(source);
    cJSON_Hooks hooks = {test_metadata_malloc, test_metadata_free};
    cJSON_InitHooks(&hooks);
    size_t failures = 0;
    for (size_t index = 1; index < 10000; ++index) {
        calls = 0;
        fail_at = index;
        MvAssetMetadata metadata = {0};
        const char *error = NULL;
        int valid = mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, &metadata, &error);
        if (!valid) {
            assert(error && !metadata.atlas && !metadata.fpg && !metadata.manifest && metadata.format == MV_ASSET_UNKNOWN);
            ++failures;
        } else assert(!error);
        mv_asset_metadata_destroy(&metadata);
        assert(!outstanding);
        if (calls < index) break;
    }
    assert(failures > 0);
    cJSON_InitHooks(NULL);
    fail_at = 0;
    calls = 0;
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    const char *paths[] = {"tests/fixtures/atlas.json", "examples/aseprite/sheet.json", "tests/fixtures/fpg/bennugd-8.fpg"};
    for (size_t i = 0; i < 3; ++i) {
        MvAssetSource *source = mv_asset_source_open(paths[i], NULL);
        check_source(source);
        mv_asset_source_destroy(source);
    }
    /* A real converted manifest supplies full palette and sprite metadata. */
    MvAssetSource *source = mv_asset_source_open(paths[2], NULL);
    MvAssetMetadata metadata = {0};
    assert(mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, &metadata, NULL));
    size_t length = mv_fpg_json_size(metadata.fpg, 1);
    char *json = malloc(length);
    assert(json && mv_fpg_copy_json(metadata.fpg, 1, json, length));
    FILE *file = fopen(argv[1], "wb");
    assert(file && fwrite(json, 1, length - 1, file) == length - 1 && fclose(file) == 0);
    free(json);
    mv_asset_metadata_destroy(&metadata);
    mv_asset_source_destroy(source);
    source = mv_asset_source_open(argv[1], NULL);
    check_source(source);
    mv_asset_source_destroy(source);
    assert(remove(argv[1]) == 0);
    return 0;
}
