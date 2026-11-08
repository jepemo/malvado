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

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "json_internal.h"
#include "malvado/atlas.h"

typedef struct { int sprite_id, duration_ms; } AnimationFrame;
typedef struct {
    char *name;
    AnimationFrame *frames;
    size_t count;
    uint64_t duration_ms;
    MvColor color;
    int has_color;
    unsigned repeats;
    int has_repeats;
} Animation;

struct MvAtlas {
    char *image;
    MvAtlasSprite *sprites;
    size_t count;
    Animation *animations;
    size_t animation_count;
    MvColor pivot_color;
    int has_pivot_color;
};

/* Reject zero escapes while allowing a literal backslash followed by u0000. */
static int
has_zero_escape(const char *json, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (json[i] != '\\')
            continue;
        if (i + 5 < length && !memcmp(json + i + 1, "u0000", 5))
            return 1;
        if (i + 1 < length)
            ++i;
    }
    return 0;
}

static char *
copy_text(const cJSON *field, size_t limit)
{
    if (!cJSON_IsString(field) || !field->valuestring)
        return NULL;
    size_t length = strlen(field->valuestring);
    if (length == 0 || length > limit)
        return NULL;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)field->valuestring[i];
        if (c < 32 || c > 126)
            return NULL;
    }
    char *copy = malloc(length + 1);
    if (copy)
        memcpy(copy, field->valuestring, length + 1);
    return copy;
}

void
mv_atlas_destroy(MvAtlas *atlas)
{
    if (!atlas)
        return;
    for (size_t i = 0; i < atlas->count; ++i)
        free((void *)atlas->sprites[i].name);
    for (size_t i = 0; i < atlas->animation_count; ++i) {
        free(atlas->animations[i].name);
        free(atlas->animations[i].frames);
    }
    free(atlas->animations);
    free(atlas->sprites);
    free(atlas->image);
    free(atlas);
}

MvAtlas *
mv_atlas_parse(const char *json, size_t length, int width, int height, const char **error)
{
    static const char *const root_fields[] = {"schema", "kind", "image", "sprites", "animations"};
    static const char *const animation_fields[] = {"name", "frames"};
    static const char *const frame_fields[] = {"sprite", "duration_ms"};
    static const char *const sprite_fields[] = {
        "id", "name", "x", "y", "width", "height", "pivot_x", "pivot_y"
    };
    if (error)
        *error = "invalid atlas metadata";
    if (!json || length == 0 || length > 1024 * 1024 || width <= 0 || height <= 0 ||
        memchr(json, '\0', length) || !json_numbers_valid(json, length))
        return NULL;
    /* cJSON strings cannot represent an embedded zero byte. */
    if (has_zero_escape(json, length))
        return NULL;
    char *input = malloc(length + 1);
    if (!input)
        return NULL;
    memcpy(input, json, length);
    input[length] = '\0';
    cJSON *root = cJSON_ParseWithLengthOpts(input, length + 1, NULL, 1);
    free(input);
    if (!root)
        return NULL;
    MvAtlas *atlas = NULL;
    int schema;
    const cJSON *kind = cJSON_GetObjectItemCaseSensitive(root, "kind");
    const cJSON *sprites = cJSON_GetObjectItemCaseSensitive(root, "sprites");
    const cJSON *animations = cJSON_GetObjectItemCaseSensitive(root, "animations");
    if (!object_fields(root, root_fields, 5) ||
        !integer_field(root, "schema", 1, 2, &schema) ||
        (schema == 1 && animations) || (schema == 2 && !cJSON_IsArray(animations)) ||
        !cJSON_IsString(kind) || strcmp(kind->valuestring, "graphics-library") != 0 ||
        !cJSON_IsArray(sprites))
        goto fail;
    int count = cJSON_GetArraySize(sprites);
    if (count < 1 || count > 4096)
        goto fail;
    atlas = calloc(1, sizeof(*atlas));
    if (!atlas)
        goto fail;
    atlas->image = copy_text(cJSON_GetObjectItemCaseSensitive(root, "image"), 511);
    atlas->sprites = calloc((size_t)count, sizeof(*atlas->sprites));
    if (!atlas->image || !atlas->sprites)
        goto fail;
    const cJSON *item;
    cJSON_ArrayForEach(item, sprites) {
        MvAtlasSprite *sprite = &atlas->sprites[atlas->count];
        if (!object_fields(item, sprite_fields, 8) ||
            !integer_field(item, "id", 1, INT_MAX, &sprite->id) ||
            !integer_field(item, "x", 0, width - 1, &sprite->region.x) ||
            !integer_field(item, "y", 0, height - 1, &sprite->region.y) ||
            !integer_field(item, "width", 1, width - sprite->region.x, &sprite->region.width) ||
            !integer_field(item, "height", 1, height - sprite->region.y, &sprite->region.height))
            goto fail;
        sprite->source_width = sprite->region.width;
        sprite->source_height = sprite->region.height;
        const cJSON *px = cJSON_GetObjectItemCaseSensitive(item, "pivot_x");
        const cJSON *py = cJSON_GetObjectItemCaseSensitive(item, "pivot_y");
        if ((px && !integer_field(item, "pivot_x", INT_MIN, INT_MAX, &sprite->pivot_x)) ||
            (py && !integer_field(item, "pivot_y", INT_MIN, INT_MAX, &sprite->pivot_y)))
            goto fail;
        sprite->name = copy_text(cJSON_GetObjectItemCaseSensitive(item, "name"), 127);
        if (!sprite->name)
            goto fail;
        ++atlas->count;
        for (size_t i = 0; i + 1 < atlas->count; ++i) {
            if (atlas->sprites[i].id == sprite->id || strcmp(atlas->sprites[i].name, sprite->name) == 0)
                goto fail;
        }
    }
    if (schema == 2) {
        int animation_count = cJSON_GetArraySize(animations);
        if (animation_count < 1 || animation_count > 256)
            goto fail;
        atlas->animations = calloc((size_t)animation_count, sizeof(*atlas->animations));
        if (!atlas->animations)
            goto fail;
        size_t total_frames = 0;
        const cJSON *entry;
        cJSON_ArrayForEach(entry, animations) {
            Animation *animation = &atlas->animations[atlas->animation_count++];
            const cJSON *frames = cJSON_GetObjectItemCaseSensitive(entry, "frames");
            if (!object_fields(entry, animation_fields, 2) || !cJSON_IsArray(frames))
                goto fail;
            int frame_count = cJSON_GetArraySize(frames);
            if (frame_count < 1 || (size_t)frame_count > 4096 - total_frames)
                goto fail;
            total_frames += (size_t)frame_count;
            animation->name = copy_text(cJSON_GetObjectItemCaseSensitive(entry, "name"), 127);
            animation->frames = calloc((size_t)frame_count, sizeof(*animation->frames));
            if (!animation->name || !animation->frames)
                goto fail;
            for (size_t i = 0; i + 1 < atlas->animation_count; ++i)
                if (strcmp(atlas->animations[i].name, animation->name) == 0)
                    goto fail;
            const cJSON *frame;
            cJSON_ArrayForEach(frame, frames) {
                AnimationFrame *current = &animation->frames[animation->count];
                MvAtlasSprite sprite;
                if (!object_fields(frame, frame_fields, 2) ||
                    !integer_field(frame, "sprite", 1, INT_MAX, &current->sprite_id) ||
                    !integer_field(frame, "duration_ms", 1, 3600000, &current->duration_ms) ||
                    !mv_atlas_find(atlas, current->sprite_id, &sprite))
                    goto fail;
                animation->duration_ms += (uint64_t)current->duration_ms;
                ++animation->count;
            }
        }
    }
    cJSON_Delete(root);
    if (error)
        *error = NULL;
    return atlas;
fail:
    mv_atlas_destroy(atlas);
    cJSON_Delete(root);
    return NULL;
}

static int
hex_digit(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int
parse_hex_color(const cJSON *field, MvColor *color)
{
    if (!cJSON_IsString(field) || !field->valuestring) return 0;
    const char *text = field->valuestring;
    size_t length = strlen(text);
    if ((length != 7 && length != 9) || text[0] != '#') return 0;
    unsigned char rgba[4] = {0, 0, 0, 255};
    for (size_t i = 0; i < (length - 1) / 2; ++i) {
        int high = hex_digit(text[1 + i * 2]);
        int low = hex_digit(text[2 + i * 2]);
        if (high < 0 || low < 0) return 0;
        rgba[i] = (unsigned char)(high * 16 + low);
    }
    *color = (MvColor){rgba[0], rgba[1], rgba[2], rgba[3]};
    return 1;
}

static int
parse_tag_repeats(const cJSON *field, unsigned *out)
{
    if (!cJSON_IsString(field) || !field->valuestring || !field->valuestring[0])
        return 0;
    unsigned value = 0;
    for (const char *p = field->valuestring; *p; ++p) {
        if (*p < '0' || *p > '9')
            return 0;
        unsigned digit = (unsigned)(*p - '0');
        if (value > (65535U - digit) / 10U)
            return 0;
        value = value * 10U + digit;
    }
    *out = value;
    return 1;
}

/* Import the supported array subset into the existing atlas representation. */
MvAtlas *
mv_atlas_parse_aseprite(const char *json, size_t length, int width, int height,
                        const char **error)
{
    static const char *const root_fields[] = {"frames", "meta"};
    static const char *const meta_fields[] = {"app", "version", "image", "format", "size", "scale", "frameTags", "layers", "slices"};
    static const char *const frame_fields[] = {"filename", "frame", "rotated", "trimmed", "spriteSourceSize", "sourceSize", "duration"};
    static const char *const rect_fields[] = {"x", "y", "w", "h"};
    static const char *const size_fields[] = {"w", "h"};
    if (error) *error = "invalid or unsupported Aseprite metadata";
    if (!json || length == 0 || length > 1024 * 1024 || width <= 0 || height <= 0 ||
        memchr(json, '\0', length) || !json_numbers_valid(json, length))
        return NULL;
    if (has_zero_escape(json, length))
        return NULL;
    char *input = malloc(length + 1);
    if (!input) return NULL;
    memcpy(input, json, length);
    input[length] = '\0';
    cJSON *root = cJSON_ParseWithLengthOpts(input, length + 1, NULL, 1);
    free(input);
    if (!root) return NULL;
    MvAtlas *atlas = NULL;
    const cJSON *meta = cJSON_GetObjectItemCaseSensitive(root, "meta");
    const cJSON *frames = cJSON_GetObjectItemCaseSensitive(root, "frames");
    const cJSON *size = cJSON_GetObjectItemCaseSensitive(meta, "size");
    int w, h;
    if (!object_fields(root, root_fields, 2) || !object_fields(meta, meta_fields, 9) ||
        !object_fields(size, size_fields, 2) ||
        !integer_field(size, "w", 1, INT_MAX, &w) || w != width ||
        !integer_field(size, "h", 1, INT_MAX, &h) || h != height ||
        !cJSON_IsArray(frames)) goto fail_aseprite;
    const cJSON *layers = cJSON_GetObjectItemCaseSensitive(meta, "layers");
    if (layers && (!cJSON_IsArray(layers) || layers->child)) {
        if (error) *error = "unsupported Aseprite layers";
        goto fail_aseprite;
    }
    const cJSON *format = cJSON_GetObjectItemCaseSensitive(meta, "format");
    const cJSON *scale = cJSON_GetObjectItemCaseSensitive(meta, "scale");
    const cJSON *app = cJSON_GetObjectItemCaseSensitive(meta, "app");
    const cJSON *version = cJSON_GetObjectItemCaseSensitive(meta, "version");
    if (format && (!cJSON_IsString(format) || strcmp(format->valuestring, "RGBA8888"))) {
        if (error) *error = "unsupported Aseprite pixel format";
        goto fail_aseprite;
    }
    if ((scale && (!cJSON_IsString(scale) || strcmp(scale->valuestring, "1"))) ||
        (app && !cJSON_IsString(app)) || (version && !cJSON_IsString(version))) goto fail_aseprite;
    int count = cJSON_GetArraySize(frames);
    if (count < 1 || count > 4096) goto fail_aseprite;
    const cJSON *tags = cJSON_GetObjectItemCaseSensitive(meta, "frameTags");
    if (tags && !cJSON_IsArray(tags)) goto fail_aseprite;
    int tag_count = tags ? cJSON_GetArraySize(tags) : 0;
    if (tag_count > 255) goto fail_aseprite;
    atlas = calloc(1, sizeof(*atlas));
    if (!atlas) goto fail_aseprite;
    atlas->image = copy_text(cJSON_GetObjectItemCaseSensitive(meta, "image"), 511);
    atlas->sprites = calloc((size_t)count, sizeof(*atlas->sprites));
    atlas->animations = calloc((size_t)tag_count + 1, sizeof(*atlas->animations));
    if (!atlas->image || !atlas->sprites || !atlas->animations) goto fail_aseprite;
    atlas->animation_count = 1;
    Animation *animation = atlas->animations;
    animation->name = malloc(sizeof("default"));
    animation->frames = calloc((size_t)count, sizeof(*animation->frames));
    if (!animation->name || !animation->frames) goto fail_aseprite;
    memcpy(animation->name, "default", sizeof("default"));
    const cJSON *entry;
    cJSON_ArrayForEach(entry, frames) {
        const cJSON *rect = cJSON_GetObjectItemCaseSensitive(entry, "frame");
        const cJSON *source = cJSON_GetObjectItemCaseSensitive(entry, "spriteSourceSize");
        const cJSON *original = cJSON_GetObjectItemCaseSensitive(entry, "sourceSize");
        MvAtlasSprite *sprite = &atlas->sprites[atlas->count];
        const cJSON *rotated = cJSON_GetObjectItemCaseSensitive(entry, "rotated");
        const cJSON *trimmed = cJSON_GetObjectItemCaseSensitive(entry, "trimmed");
        if (cJSON_IsTrue(rotated)) {
            if (error) *error = "unsupported Aseprite rotated frame";
            goto fail_aseprite;
        }
        int sx, sy, sw, sh, ow, oh, duration;
        if (!object_fields(entry, frame_fields, 7) || !object_fields(rect, rect_fields, 4) ||
            !object_fields(source, rect_fields, 4) || !object_fields(original, size_fields, 2) ||
            !cJSON_IsFalse(rotated) || !cJSON_IsBool(trimmed) ||
            !integer_field(rect, "x", 0, width - 1, &sprite->region.x) ||
            !integer_field(rect, "y", 0, height - 1, &sprite->region.y) ||
            !integer_field(rect, "w", 1, width - sprite->region.x, &sprite->region.width) ||
            !integer_field(rect, "h", 1, height - sprite->region.y, &sprite->region.height) ||
            !integer_field(source, "x", 0, INT_MAX, &sx) || !integer_field(source, "y", 0, INT_MAX, &sy) ||
            !integer_field(source, "w", 1, INT_MAX, &sw) || !integer_field(source, "h", 1, INT_MAX, &sh) ||
            !integer_field(original, "w", 1, INT_MAX, &ow) || !integer_field(original, "h", 1, INT_MAX, &oh) ||
            sw != sprite->region.width || sh != sprite->region.height ||
            sw > ow || sh > oh || sx > ow - sw || sy > oh - sh ||
            (cJSON_IsFalse(trimmed) && (sx != 0 || sy != 0 || ow != sw || oh != sh)) ||
            !integer_field(entry, "duration", 1, 3600000, &duration)) goto fail_aseprite;
        sprite->source_width = ow;
        sprite->source_height = oh;
        sprite->trim_x = sx;
        sprite->trim_y = sy;
        sprite->pivot_x = -sx;
        sprite->pivot_y = -sy;
        sprite->name = copy_text(cJSON_GetObjectItemCaseSensitive(entry, "filename"), 127);
        if (!sprite->name) goto fail_aseprite;
        sprite->id = (int)atlas->count + 1;
        ++atlas->count;
        for (size_t i = 0; i + 1 < atlas->count; ++i)
            if (!strcmp(atlas->sprites[i].name, sprite->name)) goto fail_aseprite;
        animation->frames[animation->count++] = (AnimationFrame){sprite->id, duration};
        animation->duration_ms += (uint64_t)duration;
    }
    /* A pivot key applies until the next key. */
    const cJSON *slices = cJSON_GetObjectItemCaseSensitive(meta, "slices");
    if (slices && !cJSON_IsArray(slices)) goto fail_aseprite;
    if (slices && slices->child) {
        static const char *const slice_fields[] = {"name", "color", "keys"};
        static const char *const key_fields[] = {"frame", "bounds", "pivot"};
        static const char *const point_fields[] = {"x", "y"};
        const cJSON *slice = slices->child;
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(slice, "name");
        const cJSON *keys = cJSON_GetObjectItemCaseSensitive(slice, "keys");
        const cJSON *color = cJSON_GetObjectItemCaseSensitive(slice, "color");
        if (slice->next || !object_fields(slice, slice_fields, 3) ||
            !cJSON_IsString(name) || strcmp(name->valuestring, "pivot") ||
            !cJSON_IsArray(keys) || !keys->child ||
            (color && !parse_hex_color(color, &atlas->pivot_color))) {
            if (error) *error = "invalid or unsupported Aseprite pivot slice";
            goto fail_aseprite;
        }
        atlas->has_pivot_color = color != NULL;
        const cJSON *key;
        int previous = -1;
        cJSON_ArrayForEach(key, keys) {
            const cJSON *bounds = cJSON_GetObjectItemCaseSensitive(key, "bounds");
            const cJSON *pivot = cJSON_GetObjectItemCaseSensitive(key, "pivot");
            int frame, bx, by, bw, bh, px, py;
            if (!object_fields(key, key_fields, 3) ||
                !object_fields(bounds, rect_fields, 4) || !object_fields(pivot, point_fields, 2) ||
                !integer_field(key, "frame", 0, count - 1, &frame) ||
                (previous < 0 ? frame != 0 : frame <= previous) ||
                !integer_field(bounds, "x", 0, INT_MAX, &bx) || !integer_field(bounds, "y", 0, INT_MAX, &by) ||
                !integer_field(bounds, "w", 1, INT_MAX, &bw) || !integer_field(bounds, "h", 1, INT_MAX, &bh) ||
                !integer_field(pivot, "x", INT_MIN, INT_MAX, &px) ||
                !integer_field(pivot, "y", INT_MIN, INT_MAX, &py)) {
                if (error) *error = "invalid or unsupported Aseprite pivot slice";
                goto fail_aseprite;
            }
            int end = count;
            if (key->next && !integer_field(key->next, "frame", frame + 1, count - 1, &end)) {
                if (error) *error = "invalid or unsupported Aseprite pivot slice";
                goto fail_aseprite;
            }
            previous = frame;
            for (size_t i = (size_t)frame; i < (size_t)end; ++i) {
                MvAtlasSprite *sprite = &atlas->sprites[i];
                int64_t x = (int64_t)bx + px - sprite->trim_x;
                int64_t y = (int64_t)by + py - sprite->trim_y;
                if (bw > sprite->source_width || bh > sprite->source_height ||
                    bx > sprite->source_width - bw || by > sprite->source_height - bh ||
                    x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX) {
                    if (error) *error = "invalid or unsupported Aseprite pivot slice";
                    goto fail_aseprite;
                }
                sprite->pivot_x = (int)x;
                sprite->pivot_y = (int)y;
            }
        }
    }
    static const char *const tag_fields[] = {"name", "from", "to", "direction", "color", "repeat"};
    size_t total_frames = (size_t)count;
    const cJSON *tag;
    cJSON_ArrayForEach(tag, tags) {
        int from, to;
        const cJSON *direction = cJSON_GetObjectItemCaseSensitive(tag, "direction");
        if (!object_fields(tag, tag_fields, 6) ||
            !integer_field(tag, "from", 0, count - 1, &from) ||
            !integer_field(tag, "to", from, count - 1, &to) ||
            !cJSON_IsString(direction) ||
            (strcmp(direction->valuestring, "forward") && strcmp(direction->valuestring, "reverse") &&
             strcmp(direction->valuestring, "pingpong") && strcmp(direction->valuestring, "pingpong_reverse"))) {
            if (error) *error = "invalid or unsupported Aseprite frame tag";
            goto fail_aseprite;
        }
        size_t range_count = (size_t)(to - from + 1);
        int pingpong = !strcmp(direction->valuestring, "pingpong") ||
                       !strcmp(direction->valuestring, "pingpong_reverse");
        size_t frame_count = range_count;
        if (pingpong && range_count > 2)
            frame_count += range_count - 2;
        if (frame_count > 4096 - total_frames) goto fail_aseprite;
        total_frames += frame_count;
        Animation *tag_animation = &atlas->animations[atlas->animation_count++];
        const cJSON *color = cJSON_GetObjectItemCaseSensitive(tag, "color");
        if (color && !parse_hex_color(color, &tag_animation->color)) {
            if (error) *error = "invalid Aseprite tag color";
            goto fail_aseprite;
        }
        tag_animation->has_color = color != NULL;
        const cJSON *repeat = cJSON_GetObjectItemCaseSensitive(tag, "repeat");
        if (repeat && !parse_tag_repeats(repeat, &tag_animation->repeats)) {
            if (error) *error = "invalid Aseprite tag repeat count";
            goto fail_aseprite;
        }
        tag_animation->has_repeats = repeat != NULL;
        tag_animation->name = copy_text(cJSON_GetObjectItemCaseSensitive(tag, "name"), 127);
        if (!tag_animation->name) goto fail_aseprite;
        for (size_t i = 0; i + 1 < atlas->animation_count; ++i)
            if (!strcmp(atlas->animations[i].name, tag_animation->name)) goto fail_aseprite;
        tag_animation->frames = calloc(frame_count, sizeof(*tag_animation->frames));
        if (!tag_animation->frames) goto fail_aseprite;
        int reverse = !strcmp(direction->valuestring, "reverse") ||
                      !strcmp(direction->valuestring, "pingpong_reverse");
        for (size_t i = 0; i < frame_count; ++i) {
            size_t position = i < range_count ? i : 2 * range_count - 2 - i;
            size_t source_index = (size_t)(reverse ? to - (int)position : from + (int)position);
            AnimationFrame frame = animation->frames[source_index];
            tag_animation->frames[tag_animation->count++] = frame;
            tag_animation->duration_ms += (uint64_t)frame.duration_ms;
        }
    }
    cJSON_Delete(root);
    if (error) *error = NULL;
    return atlas;
fail_aseprite:
    mv_atlas_destroy(atlas);
    cJSON_Delete(root);
    return NULL;
}

const char *
mv_atlas_image(const MvAtlas *atlas)
{
    return atlas ? atlas->image : NULL;
}

size_t
mv_atlas_count(const MvAtlas *atlas)
{
    return atlas ? atlas->count : 0;
}

int
mv_atlas_find(const MvAtlas *atlas, int id, MvAtlasSprite *sprite)
{
    if (!atlas || !sprite)
        return 0;
    for (size_t i = 0; i < atlas->count; ++i) {
        if (atlas->sprites[i].id == id) {
            *sprite = atlas->sprites[i];
            return 1;
        }
    }
    return 0;
}

int
mv_atlas_find_name(const MvAtlas *atlas, const char *name, MvAtlasSprite *sprite)
{
    if (!atlas || !name || !sprite)
        return 0;
    for (size_t i = 0; i < atlas->count; ++i) {
        if (strcmp(atlas->sprites[i].name, name) == 0) {
            *sprite = atlas->sprites[i];
            return 1;
        }
    }
    return 0;
}

int
mv_atlas_pivot_color(const MvAtlas *atlas, MvColor *out)
{
    if (!atlas || !out || !atlas->has_pivot_color) return 0;
    *out = atlas->pivot_color;
    return 1;
}

int
mv_atlas_animation_color(const MvAtlas *atlas, const char *name, MvColor *out)
{
    if (!atlas || !name || !out) return 0;
    for (size_t i = 0; i < atlas->animation_count; ++i) {
        const Animation *animation = &atlas->animations[i];
        if (!strcmp(animation->name, name) && animation->has_color) {
            *out = animation->color;
            return 1;
        }
    }
    return 0;
}

int
mv_atlas_animation_repeats(const MvAtlas *atlas, const char *name, unsigned *out)
{
    if (!atlas || !name || !out) return 0;
    for (size_t i = 0; i < atlas->animation_count; ++i) {
        const Animation *animation = &atlas->animations[i];
        if (!strcmp(animation->name, name) && animation->has_repeats) {
            *out = animation->repeats;
            return 1;
        }
    }
    return 0;
}

int
mv_atlas_animation_frame(const MvAtlas *atlas, const char *name, uint64_t elapsed_ms,
                         int loop, int *sprite_id)
{
    if (!atlas || !name || !sprite_id || (loop != 0 && loop != 1))
        return 0;
    for (size_t i = 0; i < atlas->animation_count; ++i) {
        const Animation *animation = &atlas->animations[i];
        if (strcmp(animation->name, name) != 0)
            continue;
        if (loop)
            elapsed_ms %= animation->duration_ms;
        else if (elapsed_ms >= animation->duration_ms) {
            *sprite_id = animation->frames[animation->count - 1].sprite_id;
            return 1;
        }
        for (size_t j = 0; j < animation->count; ++j) {
            if (elapsed_ms < (uint64_t)animation->frames[j].duration_ms) {
                *sprite_id = animation->frames[j].sprite_id;
                return 1;
            }
            elapsed_ms -= (uint64_t)animation->frames[j].duration_ms;
        }
    }
    return 0;
}
