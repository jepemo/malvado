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
#include "cJSON.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

struct MvAssetSource {
    char *path, *bytes;
    size_t length;
    MvAssetFormat format;
    int width, height;
};

static void
classify(MvAssetSource *source)
{
    const char *bytes = source->bytes;
    size_t length = source->length;
    if (length >= 8 && !memcmp(bytes, "\x89PNG\r\n\x1a\n", 8)) {
        source->format = MV_ASSET_PNG;
        return;
    }
    if (length >= 4 && !memcmp(bytes, "fpg\x1a", 4)) {
        source->format = MV_ASSET_FPG;
        return;
    }
    if (length > 16 * 1024 * 1024 || memchr(bytes, '\0', length)) return;
    cJSON *root = cJSON_ParseWithLengthOpts(bytes, length + 1, NULL, 1);
    if (!root) return;
    const cJSON *kind = cJSON_GetObjectItemCaseSensitive(root, "kind");
    const cJSON *meta = cJSON_GetObjectItemCaseSensitive(root, "meta");
    const cJSON *frames = cJSON_GetObjectItemCaseSensitive(root, "frames");
    if (cJSON_IsString(kind) && !meta && !frames) {
        if (!strcmp(kind->valuestring, "graphics-library")) source->format = MV_ASSET_ATLAS;
        else if (!strcmp(kind->valuestring, "malvado-fpg-8")) source->format = MV_ASSET_FPG_MANIFEST;
    } else if (!kind && meta && frames) {
        source->format = MV_ASSET_ASEPRITE;
        const cJSON *size = cJSON_GetObjectItemCaseSensitive(meta, "size");
        const cJSON *w = cJSON_GetObjectItemCaseSensitive(size, "w");
        const cJSON *h = cJSON_GetObjectItemCaseSensitive(size, "h");
        if (cJSON_IsNumber(w) && cJSON_IsNumber(h) &&
            w->valuedouble >= 1 && w->valuedouble <= INT_MAX &&
            h->valuedouble >= 1 && h->valuedouble <= INT_MAX) {
            source->width = (int)w->valuedouble;
            source->height = (int)h->valuedouble;
        }
    }
    cJSON_Delete(root);
}

void
mv_asset_source_destroy(MvAssetSource *source)
{
    if (!source) return;
    free(source->path);
    free(source->bytes);
    free(source);
}

static FILE *
open_asset_file(const char *path)
{
#ifdef _WIN32
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, NULL, 0);
    if (count <= 0) return NULL;
    wchar_t *wide = malloc((size_t)count * sizeof(*wide));
    if (!wide) return NULL;
    FILE *file = NULL;
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, count) == count)
        file = _wfopen(wide, L"rb");
    free(wide);
    return file;
#else
    return fopen(path, "rb");
#endif
}

MvAssetSource *
mv_asset_source_open(const char *path, const char **error)
{
    const char *message = "invalid asset path";
    FILE *file = NULL;
    MvAssetSource *source = NULL;
    if (!path || !*path) goto fail;
    message = "cannot open asset";
    file = open_asset_file(path);
    if (!file) goto fail;
    message = "cannot read asset (maximum 64 MiB)";
    if (fseek(file, 0, SEEK_END) != 0) goto fail;
    long size = ftell(file);
    if (size < 0 || size > 64 * 1024 * 1024 || fseek(file, 0, SEEK_SET) != 0) goto fail;
    message = "out of memory reading asset";
    source = calloc(1, sizeof(*source));
    if (!source) goto fail;
    source->path = malloc(strlen(path) + 1);
    source->bytes = malloc((size_t)size + 1);
    if (!source->path || !source->bytes) goto fail;
    strcpy(source->path, path);
    message = "cannot read asset or source changed";
    size_t count = fread(source->bytes, 1, (size_t)size, file);
    int extra = fgetc(file);
    int failed = ferror(file);
    int closed = fclose(file);
    file = NULL;
    if (count != (size_t)size || extra != EOF || failed || closed != 0) goto fail;
    source->length = count;
    source->bytes[count] = '\0';
    classify(source);
    if (error) *error = NULL;
    return source;
fail:
    if (file) fclose(file);
    mv_asset_source_destroy(source);
    if (error) *error = message;
    return NULL;
}

const char *
mv_asset_source_path(const MvAssetSource *source)
{
    return source ? source->path : NULL;
}
const char *
mv_asset_source_data(const MvAssetSource *source, size_t *length)
{
    if (!source || !length) return NULL;
    *length = source->length;
    return source->bytes;
}
MvAssetFormat
mv_asset_source_format(const MvAssetSource *source)
{
    return source ? source->format : MV_ASSET_UNKNOWN;
}
int
mv_asset_source_dimensions(const MvAssetSource *source, int *width, int *height)
{
    if (!source || !width || !height || width == height || !source->width || !source->height) return 0;
    *width = source->width;
    *height = source->height;
    return 1;
}

static size_t
image_prefix(const MvAssetSource *source, const char *image)
{
    if (image[0] == '/') return 0;
    const char *slash = strrchr(source->path, '/');
#ifdef _WIN32
    if (image[0] == '\\' || (image[0] && image[1] == ':')) return 0;
    const char *backslash = strrchr(source->path, '\\');
    if (backslash && (!slash || backslash > slash)) slash = backslash;
#endif
    return slash ? (size_t)(slash - source->path) + 1 : 0;
}
size_t
mv_asset_source_image_path_size(const MvAssetSource *source, const char *image)
{
    if (!source || !image || !*image) return 0;
    size_t prefix = image_prefix(source, image), length = strlen(image);
    if (prefix > SIZE_MAX - length - 1) return 0;
    return prefix + length + 1;
}
int
mv_asset_source_image_path(const MvAssetSource *source, const char *image, char *out, size_t capacity)
{
    size_t size = mv_asset_source_image_path_size(source, image);
    if (!size || !out || capacity < size) return 0;
    size_t prefix = image_prefix(source, image);
    memcpy(out, source->path, prefix);
    memcpy(out + prefix, image, size - prefix);
    return 1;
}
