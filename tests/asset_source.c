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

static void write_source(const char *path, const char *bytes, size_t length)
{
    FILE *file = fopen(path, "wb");
    assert(file && fwrite(bytes, 1, length, file) == length && fclose(file) == 0);
}
static void sparse_source(const char *path, long size)
{
    FILE *file = fopen(path, "wb");
    assert(file && fseek(file, size - 1, SEEK_SET) == 0);
    assert(fputc(0, file) == 0 && fclose(file) == 0);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    const char *path = argv[1], *error = NULL;
    assert(!mv_asset_source_open(NULL, &error) && error);
    assert(!mv_asset_source_open("", &error) && error);
#ifdef _WIN32
    assert(!mv_asset_source_open("invalid-\xff.json", &error) && error);
#endif
    remove(path);
    assert(!mv_asset_source_open(path, &error) && error);
    const char *samples[] = {"", "garbage", "{", "{}", "{\"kind\":\"graphics-library\"}",
        "{\"kind\":\"malvado-fpg-8\"}", "{\"frames\":[],\"meta\":{\"size\":{\"w\":8,\"h\":16}}}",
        "{\"kind\":\"graphics-library\",\"frames\":[],\"meta\":{}}", "{\"kind\":\"other\"}",
        "{\"frames\":[]}", "{\"meta\":{}}", "[]"};
    MvAssetFormat formats[] = {MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN,
        MV_ASSET_ATLAS, MV_ASSET_FPG_MANIFEST, MV_ASSET_ASEPRITE, MV_ASSET_UNKNOWN,
        MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN, MV_ASSET_UNKNOWN};
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        size_t length = strlen(samples[i]);
        write_source(path, samples[i], length);
        MvAssetSource *source = mv_asset_source_open(path, &error);
        assert(source && !error && mv_asset_source_format(source) == formats[i]);
        size_t actual = 99;
        const char *bytes = mv_asset_source_data(source, &actual);
        assert(bytes && actual == length && !memcmp(bytes, samples[i], length) && bytes[length] == 0);
        assert(!strcmp(mv_asset_source_path(source), path));
        int w = 123, h = 456;
        int dimensions = mv_asset_source_dimensions(source, &w, &h);
        if (formats[i] == MV_ASSET_ASEPRITE) assert(dimensions && w == 8 && h == 16);
        else assert(!dimensions && w == 123 && h == 456);
        assert(!mv_asset_source_dimensions(source, &w, &w));
        assert(!mv_asset_source_dimensions(source, NULL, &h));
        assert(!mv_asset_source_data(source, NULL));
        char result[4096] = "unchanged";
        assert(!mv_asset_source_image_path(source, "sheet.png", result, 1) && !strcmp(result, "unchanged"));
        assert(!mv_asset_source_image_path_size(source, NULL));
        assert(!mv_asset_source_image_path_size(source, ""));
        assert(!mv_asset_source_image_path(source, "sheet.png", NULL, sizeof(result)));
        size_t size = mv_asset_source_image_path_size(source, "sheet.png");
        assert(size && size <= sizeof(result));
        assert(mv_asset_source_image_path(source, "sheet.png", result, size));
        const char *slash = strrchr(path, '/');
        size_t prefix = slash ? (size_t)(slash - path) + 1 : 0;
        assert(!strncmp(result, path, prefix) && !strcmp(result + prefix, "sheet.png"));
        assert(mv_asset_source_image_path(source, "/absolute.png", result, sizeof(result)) && !strcmp(result, "/absolute.png"));
        /* No FILE is retained; snapshot bytes remain stable after removal. */
        assert(remove(path) == 0 && !memcmp(bytes, samples[i], length));
        mv_asset_source_destroy(source);
    }
    const char png[] = "\x89PNG\r\n\x1a\n", fpg[] = "fpg\x1a";
    const char *magic[] = {png, fpg};
    for (int i = 0; i < 2; ++i) {
        write_source(path, magic[i], i ? 4 : 8);
        MvAssetSource *source = mv_asset_source_open(path, NULL);
        assert(source && mv_asset_source_format(source) == (i ? MV_ASSET_FPG : MV_ASSET_PNG));
        mv_asset_source_destroy(source);
    }
    const char embedded[] = "{\"kind\":\"graphics-library\"}\0hidden";
    write_source(path, embedded, sizeof(embedded) - 1);
    MvAssetSource *source = mv_asset_source_open(path, NULL);
    assert(source && mv_asset_source_format(source) == MV_ASSET_UNKNOWN);
    mv_asset_source_destroy(source);
    /* JSON routing includes exactly 16 MiB, but not one byte more. */
    for (int extra = 0; extra < 2; ++extra) {
        FILE *file = fopen(path, "wb");
        const char *json = "{\"kind\":\"graphics-library\"}";
        size_t prefix = strlen(json), target = 16 * 1024 * 1024 + extra;
        assert(file && fwrite(json, 1, prefix, file) == prefix);
        char spaces[4096];
        memset(spaces, ' ', sizeof(spaces));
        while (prefix < target) {
            size_t count = target - prefix < sizeof(spaces) ? target - prefix : sizeof(spaces);
            assert(fwrite(spaces, 1, count, file) == count);
            prefix += count;
        }
        assert(fclose(file) == 0);
        source = mv_asset_source_open(path, NULL);
        assert(source && mv_asset_source_format(source) == (extra ? MV_ASSET_UNKNOWN : MV_ASSET_ATLAS));
        mv_asset_source_destroy(source);
    }
    sparse_source(path, 64 * 1024 * 1024);
    source = mv_asset_source_open(path, &error);
    assert(source && !error && mv_asset_source_format(source) == MV_ASSET_UNKNOWN);
    size_t length;
    assert(mv_asset_source_data(source, &length) && length == 64 * 1024 * 1024);
    mv_asset_source_destroy(source);
    sparse_source(path, 64 * 1024 * 1024 + 1);
    assert(!mv_asset_source_open(path, &error) && error);
    assert(remove(path) == 0);
    length = 123;
    assert(!mv_asset_source_data(NULL, &length) && length == 123);
    assert(!mv_asset_source_path(NULL) && mv_asset_source_format(NULL) == MV_ASSET_UNKNOWN);
    assert(!mv_asset_source_image_path_size(NULL, "a.png"));
    mv_asset_source_destroy(NULL);
    return 0;
}
