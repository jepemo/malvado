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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_file(const char *path, const char *bytes, size_t length)
{
    FILE *file = fopen(path, "wb");
    assert(file && fwrite(bytes, 1, length, file) == length && fclose(file) == 0);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    MvAssetMetadata out = {0};
    const char *error = NULL;
    assert(!mv_asset_metadata_parse(NULL, MV_ASSET_UNKNOWN, &out, &error) && error);
    mv_asset_metadata_destroy(NULL);
    mv_asset_metadata_destroy(&out);
    const char *paths[] = {"tests/fixtures/atlas.json", "examples/aseprite/sheet.json", "tests/fixtures/fpg/bennugd-8.fpg"};
    MvAssetFormat formats[] = {MV_ASSET_ATLAS, MV_ASSET_ASEPRITE, MV_ASSET_FPG};
    for (size_t i = 0; i < 3; ++i) {
        MvAssetSource *source = mv_asset_source_open(paths[i], &error);
        assert(source && !error);
        assert(!mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, NULL, &error) && error);
        assert(!mv_asset_metadata_parse(source, (MvAssetFormat)99, &out, &error) && error);
        assert(out.format == MV_ASSET_UNKNOWN && !out.atlas && !out.fpg && !out.manifest);
        assert(mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, &out, &error) && !error);
        assert(out.format == formats[i]);
        mv_asset_source_destroy(source);
        if (out.atlas) {
            assert(!out.fpg && !out.manifest && mv_atlas_count(out.atlas) == (i ? 3 : 2));
            MvAtlasSprite sprite;
            assert(mv_atlas_find(out.atlas, i ? 1 : 42, &sprite));
            /* Move metadata into a new caller owner before record cleanup. */
            MvAtlas *owned = out.atlas;
            out.atlas = NULL;
            mv_asset_metadata_destroy(&out);
            assert(mv_atlas_count(owned) == (i ? 3 : 2));
            mv_atlas_destroy(owned);
        } else {
            assert(out.fpg && !out.manifest && mv_fpg_count(out.fpg) == 2);
            size_t length = mv_fpg_json_size(out.fpg, 1);
            char *json = malloc(length);
            assert(json && mv_fpg_copy_json(out.fpg, 1, json, length));
            write_file(argv[1], json, length - 1);
            free(json);
            mv_asset_metadata_destroy(&out);
            source = mv_asset_source_open(argv[1], NULL);
            assert(source && mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, &out, &error) && !error);
            assert(out.format == MV_ASSET_FPG_MANIFEST && out.manifest && !out.atlas && !out.fpg);
            mv_asset_source_destroy(source);
            assert(remove(argv[1]) == 0 && mv_fpg_manifest_count(out.manifest) == 2);
            MvFpgManifestSprite sprite;
            assert(mv_fpg_manifest_sprite(out.manifest, 7, &sprite) && sprite.width == 2);
        }
        mv_asset_metadata_destroy(&out);
        mv_asset_metadata_destroy(&out);
        assert(out.format == MV_ASSET_UNKNOWN && !out.atlas && !out.fpg && !out.manifest);
    }
    const char *invalid[] = {"{\"kind\":\"graphics-library\"}", "{}", "fpg\x1a", "\x89PNG\r\n\x1a\n"};
    for (size_t i = 0; i < 4; ++i) {
        write_file(argv[1], invalid[i], strlen(invalid[i]));
        MvAssetSource *source = mv_asset_source_open(argv[1], NULL);
        assert(source);
        for (int format = MV_ASSET_UNKNOWN; format <= MV_ASSET_FPG_MANIFEST; ++format) {
            assert(!mv_asset_metadata_parse(source, (MvAssetFormat)format, &out, &error) && error);
            assert(out.format == MV_ASSET_UNKNOWN && !out.atlas && !out.fpg && !out.manifest);
        }
        mv_asset_source_destroy(source);
    }
    assert(remove(argv[1]) == 0);
    return 0;
}
