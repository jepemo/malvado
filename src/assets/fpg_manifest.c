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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malvado/fpg.h"
#include "json_internal.h"

typedef struct {
    MvFpgManifestSprite sprite;
    MvFpgPoint *points;
} ManifestEntry;

struct MvFpgManifest {
    uint8_t palette[768], gamma[576];
    int transparent_zero;
    size_t count;
    ManifestEntry *entries;
};

static int
hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int
read_hex(const cJSON *object, const char *name, uint8_t *out, size_t size)
{
    const cJSON *field = cJSON_GetObjectItemCaseSensitive(object, name);
    if (!cJSON_IsString(field) || !field->valuestring || strlen(field->valuestring) != size * 2)
        return 0;
    for (size_t i = 0; i < size; ++i) {
        int high = hex_digit(field->valuestring[i * 2]);
        int low = hex_digit(field->valuestring[i * 2 + 1]);
        if (high < 0 || low < 0) return 0;
        out[i] = (uint8_t)(high * 16 + low);
    }
    return 1;
}

void
mv_fpg_manifest_destroy(MvFpgManifest *manifest)
{
    if (!manifest) return;
    for (size_t i = 0; i < manifest->count; ++i)
        free(manifest->entries[i].points);
    free(manifest->entries);
    free(manifest);
}

MvFpgManifest *
mv_fpg_manifest_parse(const char *json, size_t length, const char **error)
{
    static const char *const root_fields[] = {"schema", "kind", "transparent_zero", "palette_hex", "gamma_hex", "sprites"};
    static const char *const sprite_fields[] = {"id", "image", "width", "height", "name_hex", "filename_hex", "points"};
    static const char *const point_fields[] = {"x", "y", "defined"};
    if (error) *error = "invalid or unsupported FPG manifest";
    if (!json || !length || length > 16U * 1024U * 1024U ||
        memchr(json, 0, length) || !json_numbers_valid(json, length)) return NULL;
    char *copy = malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, json, length);
    copy[length] = 0;
    cJSON *root = cJSON_ParseWithLengthOpts(copy, length + 1, NULL, 1);
    free(copy);
    if (!root) return NULL;
    MvFpgManifest *result = NULL;
    int schema;
    const cJSON *kind = cJSON_GetObjectItemCaseSensitive(root, "kind");
    const cJSON *transparent = cJSON_GetObjectItemCaseSensitive(root, "transparent_zero");
    const cJSON *sprites = cJSON_GetObjectItemCaseSensitive(root, "sprites");
    if (!object_fields(root, root_fields, 6) || !integer_field(root, "schema", 1, 1, &schema) ||
        !cJSON_IsString(kind) || strcmp(kind->valuestring, "malvado-fpg-8") ||
        !cJSON_IsBool(transparent) || !cJSON_IsArray(sprites)) goto fail;
    int count = cJSON_GetArraySize(sprites);
    if (count < 1 || count > 999) goto fail;
    result = calloc(1, sizeof(*result));
    if (!result) goto fail;
    result->transparent_zero = cJSON_IsTrue(transparent);
    if (!read_hex(root, "palette_hex", result->palette, sizeof(result->palette)) ||
        !read_hex(root, "gamma_hex", result->gamma, sizeof(result->gamma))) goto fail;
    for (size_t i = 0; i < sizeof(result->palette); ++i)
        if (result->palette[i] > 63) goto fail;
    result->entries = calloc((size_t)count, sizeof(*result->entries));
    if (!result->entries) goto fail;
    unsigned char used[1000] = {0};
    const cJSON *item;
    cJSON_ArrayForEach(item, sprites) {
        ManifestEntry *entry = &result->entries[result->count++];
        MvFpgManifestSprite *sprite = &entry->sprite;
        const cJSON *image = cJSON_GetObjectItemCaseSensitive(item, "image");
        const cJSON *points = cJSON_GetObjectItemCaseSensitive(item, "points");
        if (!object_fields(item, sprite_fields, 7) ||
            !integer_field(item, "id", 1, 999, &sprite->id) || used[sprite->id] ||
            !integer_field(item, "width", 1, 8192, &sprite->width) ||
            !integer_field(item, "height", 1, 8192, &sprite->height) ||
            !cJSON_IsString(image) || !cJSON_IsArray(points) ||
            !read_hex(item, "name_hex", sprite->name, sizeof(sprite->name)) ||
            !read_hex(item, "filename_hex", sprite->filename, sizeof(sprite->filename))) goto fail;
        used[sprite->id] = 1;
        char expected[16];
        snprintf(expected, sizeof(expected), "%d.png", sprite->id);
        if (strcmp(image->valuestring, expected)) goto fail;
        int point_count = cJSON_GetArraySize(points);
        if (point_count > 4096) goto fail;
        sprite->point_count = (size_t)point_count;
        if (point_count) {
            entry->points = calloc((size_t)point_count, sizeof(*entry->points));
            if (!entry->points) goto fail;
        }
        size_t index = 0;
        const cJSON *point;
        cJSON_ArrayForEach(point, points) {
            MvFpgPoint *p = &entry->points[index++];
            const cJSON *defined = cJSON_GetObjectItemCaseSensitive(point, "defined");
            if (!object_fields(point, point_fields, 3) ||
                !integer_field(point, "x", -32768, 32767, &p->x) ||
                !integer_field(point, "y", -32768, 32767, &p->y) || !cJSON_IsBool(defined)) goto fail;
            p->defined = cJSON_IsTrue(defined);
            if (p->defined != !(p->x == -1 && p->y == -1)) goto fail;
        }
    }
    cJSON_Delete(root);
    if (error) *error = NULL;
    return result;
fail:
    cJSON_Delete(root);
    mv_fpg_manifest_destroy(result);
    return NULL;
}

size_t mv_fpg_manifest_count(const MvFpgManifest *m) { return m ? m->count : 0; }
int mv_fpg_manifest_transparent_zero(const MvFpgManifest *m) { return m ? m->transparent_zero : -1; }
const uint8_t *mv_fpg_manifest_palette(const MvFpgManifest *m) { return m ? m->palette : NULL; }
const uint8_t *mv_fpg_manifest_gamma(const MvFpgManifest *m) { return m ? m->gamma : NULL; }

int
mv_fpg_manifest_at(const MvFpgManifest *m, size_t index, MvFpgManifestSprite *out)
{
    if (!m || !out || index >= m->count) return 0;
    *out = m->entries[index].sprite;
    return 1;
}

static const ManifestEntry *
find_manifest_entry(const MvFpgManifest *m, int id)
{
    if (!m) return NULL;
    for (size_t i = 0; i < m->count; ++i)
        if (m->entries[i].sprite.id == id) return &m->entries[i];
    return NULL;
}

int
mv_fpg_manifest_sprite(const MvFpgManifest *m, int id, MvFpgManifestSprite *out)
{
    const ManifestEntry *entry = find_manifest_entry(m, id);
    if (!entry || !out) return 0;
    *out = entry->sprite;
    return 1;
}

int
mv_fpg_manifest_point(const MvFpgManifest *m, int id, size_t index, MvFpgPoint *out)
{
    const ManifestEntry *entry = find_manifest_entry(m, id);
    if (!entry || !out || index >= entry->sprite.point_count) return 0;
    *out = entry->points[index];
    return 1;
}

int
mv_fpg_manifest_pivot(const MvFpgManifest *library, int id, MvFpgPoint *out)
{
    MvFpgManifestSprite sprite;
    if (!out || !mv_fpg_manifest_sprite(library, id, &sprite))
        return 0;
    MvFpgPoint point = {0};
    (void)mv_fpg_manifest_point(library, id, 0, &point);
    if (!point.defined)
        point.x = point.y = 0;
    point.defined = 1;
    *out = point;
    return 1;
}
