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

#include "malvado/asset.h"
#include <limits.h>

void
mv_asset_metadata_destroy(MvAssetMetadata *metadata)
{
    if (!metadata) return;
    mv_atlas_destroy(metadata->atlas);
    mv_fpg_destroy(metadata->fpg);
    mv_fpg_manifest_destroy(metadata->manifest);
    *metadata = (MvAssetMetadata){0};
}

int
mv_asset_metadata_parse(const MvAssetSource *source, MvAssetFormat format,
                        MvAssetMetadata *out, const char **error)
{
    const char *message = "invalid asset metadata arguments";
    if (!source || !out) goto fail;
    if (format == MV_ASSET_UNKNOWN) format = mv_asset_source_format(source);
    MvAssetMetadata value = {.format = format};
    size_t length;
    const char *bytes = mv_asset_source_data(source, &length);
    int width = INT_MAX, height = INT_MAX;
    (void)mv_asset_source_dimensions(source, &width, &height);
    switch (format) {
    case MV_ASSET_ATLAS:
        value.atlas = mv_atlas_parse(bytes, length, width, height, &message);
        break;
    case MV_ASSET_ASEPRITE:
        value.atlas = mv_atlas_parse_aseprite(bytes, length, width, height, &message);
        break;
    case MV_ASSET_FPG:
        value.fpg = mv_fpg_parse(bytes, length, &message);
        break;
    case MV_ASSET_FPG_MANIFEST:
        value.manifest = mv_fpg_manifest_parse(bytes, length, &message);
        break;
    case MV_ASSET_PNG:
        message = "PNG sources have no library metadata";
        goto fail;
    default:
        message = "unknown or ambiguous asset schema/format";
        goto fail;
    }
    if (!value.atlas && !value.fpg && !value.manifest) goto fail;
    *out = value;
    if (error) *error = NULL;
    return 1;
fail:
    if (error) *error = message ? message : "could not parse asset metadata";
    return 0;
}
