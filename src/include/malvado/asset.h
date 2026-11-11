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

#ifndef MALVADO_ASSET_H
#define MALVADO_ASSET_H
#include <stddef.h>
#include "malvado/atlas.h"
#include "malvado/fpg.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MV_ASSET_UNKNOWN, MV_ASSET_PNG, MV_ASSET_FPG, MV_ASSET_ATLAS,
    MV_ASSET_ASEPRITE, MV_ASSET_FPG_MANIFEST
} MvAssetFormat;
typedef struct MvAssetSource MvAssetSource;

/* Read one file of at most 64 MiB into owned memory and copy its input path.
 * Return NULL on invalid path, I/O or allocation failure. Optional error receives
 * a static message, or NULL on success. Close the FILE before returning.
 * Content detection is routing only, not full format/image validation. JSON
 * routing is limited to 16 MiB and rejects embedded NUL and mixed dialect keys.
 * Unknown content or failed JSON routing (including parser allocation failure)
 * is a successful snapshot with MV_ASSET_UNKNOWN. No backend,
 * context, resource manager, image loading or native importer is implicit.
 * Serialize calls with other native JSON parsing on the same host. */
MvAssetSource *mv_asset_source_open(const char *path, const char **error);
void mv_asset_source_destroy(MvAssetSource *source);
/* Borrowed input path and bytes remain valid until destruction. Bytes have an
 * extra NUL after length, which is not part of the source. Invalid arguments
 * return NULL and leave length unchanged. */
const char *mv_asset_source_path(const MvAssetSource *source);
const char *mv_asset_source_data(const MvAssetSource *source, size_t *length);
MvAssetFormat mv_asset_source_format(const MvAssetSource *source);
/* Copy advisory Aseprite meta.size dimensions for metadata parsing. This does
 * not validate the complete schema or image. Return 0 with unchanged outputs
 * when absent/invalid, or for NULL/aliased outputs. */
int mv_asset_source_dimensions(const MvAssetSource *source, int *width, int *height);
/* Join a referenced image to the source directory using host path rules.
 * Absolute references pass through; relative source paths stay relative.
 * Size includes NUL; 0 means invalid arguments or overflow. Copy returns 1,
 * or 0 without writing for insufficient capacity/invalid pointers. Output must
 * not overlap source storage or image. No file is opened by these operations. */
size_t mv_asset_source_image_path_size(const MvAssetSource *source, const char *image);
int mv_asset_source_image_path(const MvAssetSource *source, const char *image, char *out, size_t capacity);

/* Owned library metadata. Initialize to {0}. Success has exactly one non-NULL
 * pointer; format distinguishes native atlas and Aseprite. Do not shallow-copy
 * owning records. To move a pointer into another owner, set its field to NULL.
 * Destroy releases remaining pointers once and resets the record to {0}.
 * Metadata owns no image, device, context or source reference. */
typedef struct {
    MvAssetFormat format;
    MvAtlas *atlas;
    MvFpg *fpg;
    MvFpgManifest *manifest;
} MvAssetMetadata;
/* Select and call the existing complete library importer. UNKNOWN uses the
 * source hint; explicit formats override it. PNG/unknown/invalid formats fail.
 * Atlas bounds use advisory Aseprite dimensions or INT_MAX without an image;
 * hosts must validate against actual image dimensions before rendering.
 * Return 1 and transfer copied metadata into an empty out record, or 0 with
 * unchanged out and a static optional error. The source is borrowed only during
 * the call. No partial metadata survives failure. Destroy old out data before
 * reuse. Serialize with other native JSON parsing. Owner-thread only. */
int mv_asset_metadata_parse(const MvAssetSource *source, MvAssetFormat format,
                            MvAssetMetadata *out, const char **error);
void mv_asset_metadata_destroy(MvAssetMetadata *metadata);
#ifdef __cplusplus
}
#endif
#endif
