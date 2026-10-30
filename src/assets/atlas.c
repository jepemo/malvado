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
#include "malvado/atlas.h"

typedef struct { int sprite_id, duration_ms; } AnimationFrame;
typedef struct {
    char *name;
    AnimationFrame *frames;
    size_t count;
    uint64_t duration_ms;
} Animation;

struct MvAtlas {
    char *image;
    MvAtlasSprite *sprites;
    size_t count;
    Animation *animations;
    size_t animation_count;
};

static int
json_numbers_valid(const char *text, size_t length)
{
    size_t i = 0;
    while (i < length) {
        if (text[i] == '"') {
            ++i;
            while (i < length && text[i] != '"') {
                if (text[i] == '\\' && i + 1 < length)
                    ++i;
                ++i;
            }
            if (i < length)
                ++i;
            continue;
        }
        if (text[i] != '-' && (text[i] < '0' || text[i] > '9')) {
            ++i;
            continue;
        }
        if (text[i] == '-')
            ++i;
        if (i == length || text[i] < '0' || text[i] > '9')
            return 0;
        if (text[i++] == '0') {
            if (i < length && text[i] >= '0' && text[i] <= '9')
                return 0;
        } else {
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && text[i] == '.') {
            ++i;
            if (i == length || text[i] < '0' || text[i] > '9')
                return 0;
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && (text[i] == 'e' || text[i] == 'E')) {
            ++i;
            if (i < length && (text[i] == '+' || text[i] == '-'))
                ++i;
            if (i == length || text[i] < '0' || text[i] > '9')
                return 0;
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && text[i] != ',' && text[i] != ']' && text[i] != '}' &&
            text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
            return 0;
    }
    return 1;
}

static int
object_fields(const cJSON *object, const char *const *allowed, size_t count)
{
    if (!cJSON_IsObject(object))
        return 0;
    for (const cJSON *field = object->child; field; field = field->next) {
        size_t i;
        for (i = 0; i < count && strcmp(field->string, allowed[i]) != 0; ++i) {}
        if (i == count)
            return 0;
        for (const cJSON *other = field->next; other; other = other->next) {
            if (strcmp(field->string, other->string) == 0)
                return 0;
        }
    }
    return 1;
}

static int
integer_field(const cJSON *object, const char *name, int minimum, int maximum, int *value)
{
    const cJSON *field = cJSON_GetObjectItemCaseSensitive(object, name);
    if (!cJSON_IsNumber(field) || !isfinite(field->valuedouble) ||
        field->valuedouble < minimum || field->valuedouble > maximum ||
        floor(field->valuedouble) != field->valuedouble)
        return 0;
    *value = (int)field->valuedouble;
    return 1;
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
    for (size_t i = 0; i + 5 < length; ++i) {
        if (json[i] == '\\' && json[i + 1] == 'u' &&
            json[i + 2] == '0' && json[i + 3] == '0' &&
            json[i + 4] == '0' && json[i + 5] == '0')
            return NULL;
    }
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
