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
#include "malvado/fpg.h"

static void
write_u32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static void
invalid_manifest_change(const char *json, const char *old, const char *replacement)
{
    const char *match = strstr(json, old);
    assert(match);
    size_t capacity = strlen(json) + strlen(replacement) + 1;
    char *changed = malloc(capacity);
    assert(changed);
    int written = snprintf(changed, capacity, "%.*s%s%s", (int)(match - json), json,
                           replacement, match + strlen(old));
    assert(written > 0 && (size_t)written < capacity);
    const char *error = NULL;
    assert(!mv_fpg_manifest_parse(changed, (size_t)written, &error) && error);
    free(changed);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    uint8_t data[2048];
    size_t length = fread(data, 1, sizeof(data), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    const char *error = NULL;
    MvFpg *fpg = mv_fpg_parse(data, length, &error);
    assert(fpg && !error && mv_fpg_count(fpg) == 2);
    const uint8_t *palette = mv_fpg_palette(fpg);
    assert(palette && palette[3] == 63 && palette[4] == 0 && palette[7] == 63);
    assert(mv_fpg_gamma(fpg) && mv_fpg_gamma(fpg)[575] == 0);
    MvFpgSprite sprite;
    assert(mv_fpg_at(fpg, 0, &sprite) && sprite.id == 7);
    assert(mv_fpg_at(fpg, 1, &sprite) && sprite.id == 42);
    assert(!mv_fpg_at(fpg, 2, &sprite) && sprite.id == 42);
    assert(mv_fpg_sprite(fpg, 7, &sprite));
    assert(sprite.width == 2 && sprite.height == 2 && !memcmp(sprite.name, "red", 4));
    assert(sprite.point_count == 2 && sprite.pixels[0] == 0 && sprite.pixels[1] == 1);
    MvFpgPoint point;
    assert(mv_fpg_point(fpg, 7, 0, &point) && point.defined && point.x == 1 && point.y == 0);
    assert(mv_fpg_point(fpg, 7, 1, &point) && !point.defined);
    assert(!mv_fpg_point(fpg, 7, 2, &point) && !point.defined);
    assert(!mv_fpg_sprite(fpg, 8, &sprite) && sprite.id == 7);
    uint8_t rgba[16];
    memset(rgba, 0x5a, sizeof(rgba));
    assert(!mv_fpg_copy_rgba(fpg, 7, 1, rgba, 15) && rgba[0] == 0x5a);
    assert(!mv_fpg_copy_rgba(fpg, 99, 1, rgba, sizeof(rgba)) && rgba[0] == 0x5a);
    assert(!mv_fpg_copy_rgba(fpg, 7, 2, rgba, sizeof(rgba)) && rgba[0] == 0x5a);
    assert(!mv_fpg_copy_rgba(fpg, 7, 1, NULL, sizeof(rgba)));
    assert(mv_fpg_copy_rgba(fpg, 7, 1, rgba, sizeof(rgba)));
    assert(rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 0);
    assert(rgba[4] == 252 && rgba[5] == 0 && rgba[6] == 0 && rgba[7] == 255);
    assert(mv_fpg_copy_rgba(fpg, 42, 0, rgba, sizeof(rgba)));
    assert(rgba[0] == 0 && rgba[1] == 252 && rgba[2] == 0 && rgba[3] == 255);
    assert(mv_fpg_copy_rgba(fpg, 7, 0, rgba, sizeof(rgba)) && rgba[3] == 255);
    size_t json_size = mv_fpg_json_size(fpg, 1);
    assert(json_size > 1 && !mv_fpg_json_size(NULL, 1) && !mv_fpg_json_size(fpg, 2));
    char *json = malloc(json_size);
    assert(json);
    memset(json, 0x5a, json_size);
    assert(!mv_fpg_copy_json(fpg, 1, json, json_size - 1) && json[0] == 0x5a);
    assert(!mv_fpg_copy_json(fpg, 2, json, json_size) && json[0] == 0x5a);
    assert(!mv_fpg_copy_json(fpg, 1, NULL, json_size));
    assert(mv_fpg_copy_json(fpg, 1, json, json_size));
    assert(strlen(json) + 1 == json_size && strstr(json, "\"id\":7"));
    assert(mv_fpg_manifest_transparent_zero(NULL) == -1);
    assert(mv_fpg_manifest_count(NULL) == 0 && !mv_fpg_manifest_palette(NULL));
    MvFpgManifest *manifest = mv_fpg_manifest_parse(json, json_size - 1, &error);
    assert(manifest && !error && mv_fpg_manifest_count(manifest) == 2);
    assert(mv_fpg_manifest_transparent_zero(manifest) == 1);
    assert(!memcmp(mv_fpg_manifest_palette(manifest), mv_fpg_palette(fpg), 768));
    assert(!memcmp(mv_fpg_manifest_gamma(manifest), mv_fpg_gamma(fpg), 576));
    MvFpgManifestSprite metadata;
    assert(mv_fpg_manifest_at(manifest, 1, &metadata) && metadata.id == 42);
    assert(!mv_fpg_manifest_at(manifest, 2, &metadata) && metadata.id == 42);
    assert(mv_fpg_manifest_sprite(manifest, 7, &metadata));
    assert(metadata.width == 2 && metadata.height == 2 && metadata.point_count == 2);
    assert(!memcmp(metadata.name, "red", 4));
    assert(!mv_fpg_manifest_sprite(manifest, 8, &metadata) && metadata.id == 7);
    assert(mv_fpg_manifest_point(manifest, 7, 0, &point) && point.x == 1 && point.defined);
    assert(mv_fpg_manifest_point(manifest, 7, 1, &point) && !point.defined);
    assert(!mv_fpg_manifest_point(manifest, 7, 2, &point) && !point.defined);
    memset(json, 'x', json_size - 1);
    assert(mv_fpg_manifest_sprite(manifest, 7, &metadata) && metadata.id == 7);
    mv_fpg_manifest_destroy(manifest);
    mv_fpg_manifest_destroy(NULL);
    assert(mv_fpg_copy_json(fpg, 1, json, json_size));
    invalid_manifest_change(json, "\"schema\":1", "\"schema\":\v1");
    invalid_manifest_change(json, "\"schema\":1", "\"schema\":\f1");
    invalid_manifest_change(json, "\"schema\":1", "\"schema\":2");
    invalid_manifest_change(json, "\"schema\":1", "\"schema\":1,\"schema\":1");
    invalid_manifest_change(json, "\"image\":\"7.png\"", "\"image\":\"../7.png\"");
    invalid_manifest_change(json, "\"id\":42", "\"id\":7");
    invalid_manifest_change(json, "\"defined\":false", "\"defined\":true");
    invalid_manifest_change(json, "\"transparent_zero\":true", "\"transparent_zero\":1");
    invalid_manifest_change(json, "\"kind\":\"malvado-fpg-8\"", "\"kind\":\"malvado-fpg-8\\u0000x\"");
    invalid_manifest_change(json, "\"palette_hex\":\"", "\"palette_hex\":\"zz");
    assert(!mv_fpg_manifest_parse(json, json_size, &error) && error);
    assert(!mv_fpg_manifest_parse(NULL, 1, &error) && error);
    assert(!mv_fpg_manifest_parse(json, 16U * 1024U * 1024U + 1, &error) && error);

    free(json);
    size_t png_size = mv_fpg_png_size(fpg, 7, 1);
    assert(png_size > 0 && mv_fpg_png_size(fpg, 7, 0) + 13 == png_size);
    assert(!mv_fpg_png_size(fpg, 999, 1) && !mv_fpg_png_size(fpg, 7, 2));
    uint8_t *png = malloc(png_size);
    assert(png);
    memset(png, 0x5a, png_size);
    assert(!mv_fpg_copy_png(fpg, 7, 1, png, png_size - 1) && png[0] == 0x5a);
    assert(!mv_fpg_copy_png(fpg, 999, 1, png, png_size) && png[0] == 0x5a);
    assert(mv_fpg_copy_png(fpg, 7, 1, png, png_size));
    assert(!memcmp(png, "\x89PNG\r\n\x1a\n", 8));
    free(png);
    data[8 + 3] = 0;
    assert(mv_fpg_palette(fpg)[3] == 63);
    mv_fpg_destroy(fpg);
    mv_fpg_destroy(NULL);
    data[8 + 3] = 63;
    for (size_t i = 0; i < length; ++i) {
        /* A complete first record is itself a valid one-sprite library. */
        if (i == 1428) continue;
        assert(!mv_fpg_parse(data, i, &error) && error);
    }
    assert(!mv_fpg_parse(NULL, length, &error));
    assert(!mv_fpg_parse(data, 64U * 1024U * 1024U + 1, &error));
    data[7] = 1;
    assert(!mv_fpg_parse(data, length, &error));
    data[7] = 0;
    data[0] = 'x';
    assert(!mv_fpg_parse(data, length, &error));
    data[0] = 'f';
    data[8] = 64;
    assert(!mv_fpg_parse(data, length, &error));
    data[8] = 0;
    const size_t first = 8 + 768 + 576;
    write_u32(data + first + 52, 0);
    assert(!mv_fpg_parse(data, length, &error));
    write_u32(data + first + 52, UINT32_MAX);
    assert(!mv_fpg_parse(data, length, &error));
    write_u32(data + first + 52, 2);
    write_u32(data + first + 60, 4097);
    assert(!mv_fpg_parse(data, length, &error));
    write_u32(data + first + 60, 2);
    write_u32(data + first + 4, 1);
    assert(!mv_fpg_parse(data, length, &error));
    write_u32(data + first + 4, 0);
    write_u32(data + 1428, 7);
    assert(!mv_fpg_parse(data, length, &error));
    return 0;
}
