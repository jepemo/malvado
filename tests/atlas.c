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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "malvado/atlas.h"

static void
invalid(const char *json)
{
    const char *error = NULL;
    MvAtlas *atlas = mv_atlas_parse(json, strlen(json), 8, 8, &error);
    assert(!atlas && error);
}

static void
invalid_aseprite_change(const char *json, const char *old, const char *replacement)
{
    char changed[4096];
    const char *match = strstr(json, old);
    assert(match);
    size_t prefix = (size_t)(match - json);
    int written = snprintf(changed, sizeof(changed), "%.*s%s%s", (int)prefix, json,
                           replacement, match + strlen(old));
    assert(written > 0 && (size_t)written < sizeof(changed));
    const char *error = NULL;
    assert(!mv_atlas_parse_aseprite(changed, (size_t)written, 8, 8, &error) && error);
}

int
main(int argc, char **argv)
{
    assert(argc == 7);
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    char buffer[2048];
    size_t length = fread(buffer, 1, sizeof(buffer), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    const char *error = "not cleared";
    MvAtlas *atlas = mv_atlas_parse(buffer, length, 8, 8, &error);
    assert(atlas && !error && mv_atlas_count(atlas) == 2);
    assert(strcmp(mv_atlas_image(atlas), "red.png") == 0);
    MvAtlasSprite sprite;
    assert(mv_atlas_find(atlas, 42, &sprite));
    assert(!strcmp(sprite.name, "green") && sprite.region.x == 4 && sprite.region.width == 4);
    assert(sprite.pivot_x == 2 && sprite.pivot_y == 4);
    assert(mv_atlas_find(atlas, 7, &sprite) && sprite.pivot_x == 0 && sprite.pivot_y == 0);
    assert(sprite.source_width == 4 && sprite.source_height == 8 && sprite.trim_x == 0 && sprite.trim_y == 0);
    assert(!mv_atlas_find(atlas, 8, &sprite));
    MvColor absent = {1, 2, 3, 4};
    assert(!mv_atlas_pivot_color(atlas, &absent) && absent.r == 1 && absent.a == 4);
    assert(mv_atlas_find_name(atlas, "green", &sprite) && sprite.id == 42);
    assert(!mv_atlas_find_name(atlas, "Green", &sprite) && sprite.id == 42);
    assert(!mv_atlas_find_name(atlas, NULL, &sprite) && sprite.id == 42);
    assert(!mv_atlas_find_name(NULL, "green", &sprite) && sprite.id == 42);
    assert(!mv_atlas_find_name(atlas, "green", NULL));
    mv_atlas_destroy(atlas);
    mv_atlas_destroy(NULL);

    file = fopen(argv[2], "rb");
    assert(file);
    length = fread(buffer, 1, sizeof(buffer), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    atlas = mv_atlas_parse(buffer, length, 8, 8, &error);
    assert(atlas && !error);
    int frame = -1;
    assert(mv_atlas_animation_frame(atlas, "flash", 0, 1, &frame) && frame == 7);
    assert(mv_atlas_animation_frame(atlas, "flash", 99, 1, &frame) && frame == 7);
    assert(mv_atlas_animation_frame(atlas, "flash", 100, 1, &frame) && frame == 42);
    assert(mv_atlas_animation_frame(atlas, "flash", 300, 1, &frame) && frame == 7);
    assert(mv_atlas_animation_frame(atlas, "flash", UINT64_MAX, 0, &frame) && frame == 42);
    assert(!mv_atlas_animation_frame(atlas, "missing", 0, 1, &frame) && frame == 42);
    assert(!mv_atlas_animation_frame(atlas, "flash", 0, 2, &frame) && frame == 42);
    assert(!mv_atlas_animation_frame(atlas, "flash", 0, 1, NULL));
    mv_atlas_destroy(atlas);

    file = fopen(argv[3], "rb");
    assert(file);
    length = fread(buffer, 1, sizeof(buffer), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    assert(length < sizeof(buffer));
    buffer[length] = '\0';
    atlas = mv_atlas_parse_aseprite(buffer, length, 8, 8, &error);
    assert(atlas && !error && mv_atlas_count(atlas) == 2);
    assert(!strcmp(mv_atlas_image(atlas), "red.png"));
    assert(mv_atlas_find_name(atlas, "two", &sprite) && sprite.id == 2);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.region.x == 4 &&
           sprite.region.width == 4 && !strcmp(sprite.name, "two"));
    assert(mv_atlas_animation_frame(atlas, "default", 99, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "default", 100, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "default", 300, 1, &frame) && frame == 1);
    mv_atlas_destroy(atlas);
    assert(!mv_atlas_parse_aseprite(buffer, length, 7, 8, &error) && error);
    const char *hash = "{\"frames\":{},\"meta\":{\"image\":\"red.png\",\"size\":{\"w\":8,\"h\":8}}}";
    assert(!mv_atlas_parse_aseprite(hash, strlen(hash), 8, 8, &error));
    assert(error && !strcmp(error, "unsupported Aseprite JSON hash; export JSON array"));

    const char *tag_start = strstr(buffer, "\"frameTags\":[]");
    assert(tag_start);
    char tagged[4096];
    const char *tags = "\"frameTags\":[{\"name\":\"walk\",\"from\":0,\"to\":1,\"direction\":\"forward\"},"
                       "{\"name\":\"back\",\"from\":0,\"to\":1,\"direction\":\"reverse\"},"
                       "{\"name\":\"still\",\"from\":1,\"to\":1,\"direction\":\"reverse\"}]";
    int tagged_length = snprintf(tagged, sizeof(tagged), "%.*s%s%s", (int)(tag_start - buffer),
                                 buffer, tags, tag_start + strlen("\"frameTags\":[]"));
    assert(tagged_length > 0 && (size_t)tagged_length < sizeof(tagged));
    atlas = mv_atlas_parse_aseprite(tagged, (size_t)tagged_length, 8, 8, &error);
    assert(atlas && !error);
    assert(mv_atlas_animation_frame(atlas, "walk", 99, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "walk", 100, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "back", 199, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "back", 200, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "back", 300, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "back", UINT64_MAX, 0, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "still", 1000, 1, &frame) && frame == 2);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(tagged, "\"direction\":\"forward\"", "\"direction\":\"unknown\"");
    invalid_aseprite_change(tagged, "\"from\":0", "\"from\":2");
    invalid_aseprite_change(tagged, "\"to\":1", "\"to\":2");
    invalid_aseprite_change(tagged, "\"name\":\"walk\"", "\"name\":\"default\"");
    invalid_aseprite_change(tagged, "\"name\":\"back\"", "\"name\":\"walk\"");
    invalid_aseprite_change(tagged, "\"direction\":\"forward\"", "\"direction\":\"forward\",\"repeat\":\"-2\"");
    invalid_aseprite_change(buffer, "\"trimmed\":false", "\"trimmed\":1");
    invalid_aseprite_change(buffer, "\"frameTags\":[]", "\"frameTags\":[{}]");
    invalid_aseprite_change(buffer, "\"layers\":[]", "\"layers\":[{}]");
    invalid_aseprite_change(buffer, "\"duration\":100", "\"duration\":0");
    invalid_aseprite_change(buffer, "\"filename\":\"two\"", "\"filename\":\"one\"");
    const char *format_field = strstr(buffer, "\"format\":\"RGBA8888\"");
    assert(format_field);
    char bad_format[4096];
    int bad_format_length = snprintf(bad_format, sizeof(bad_format), "%.*s%s%s",
        (int)(format_field - buffer), buffer, "\"format\":\"I8\"",
        format_field + strlen("\"format\":\"RGBA8888\""));
    assert(bad_format_length > 0 && (size_t)bad_format_length < sizeof(bad_format));
    assert(!mv_atlas_parse_aseprite(bad_format, (size_t)bad_format_length, 8, 8, &error));
    assert(error && !strcmp(error, "unsupported Aseprite pixel format"));
    const char *scale_field = strstr(buffer, "\"scale\":\"1\"");
    assert(scale_field);
    char bad_scale[4096];
    int bad_scale_length = snprintf(bad_scale, sizeof(bad_scale), "%.*s%s%s",
        (int)(scale_field - buffer), buffer, "\"scale\":\"2\"",
        scale_field + strlen("\"scale\":\"1\""));
    assert(bad_scale_length > 0 && (size_t)bad_scale_length < sizeof(bad_scale));
    assert(!mv_atlas_parse_aseprite(bad_scale, (size_t)bad_scale_length, 8, 8, &error));
    assert(error && !strcmp(error, "unsupported Aseprite scale"));
    invalid_aseprite_change(buffer, "\"rotated\":false", "\"extra\":0,\"rotated\":false");
    invalid_aseprite_change(buffer, "\"w\":4", "\"w\":9");
    char *rotated = strstr(buffer, "\"rotated\":false");
    assert(rotated);
    memcpy(rotated + strlen("\"rotated\":"), "true ", 5);
    assert(!mv_atlas_parse_aseprite(buffer, length, 8, 8, &error) && error);
    assert(!mv_atlas_parse_aseprite("{}", 2, 8, 8, &error) && error);
    file = fopen(argv[4], "rb");
    assert(file);
    length = fread(buffer, 1, sizeof(buffer) - 1, file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    buffer[length] = '\0';
    atlas = mv_atlas_parse_aseprite(buffer, length, 8, 8, &error);
    assert(atlas && !error && mv_atlas_find(atlas, 1, &sprite));
    assert(sprite.region.x == 4 && sprite.region.width == 4 && sprite.region.height == 4);
    assert(sprite.source_width == 8 && sprite.source_height == 8);
    assert(sprite.trim_x == 2 && sprite.trim_y == 3 && sprite.pivot_x == -2 && sprite.pivot_y == -3);
    assert(mv_atlas_animation_frame(atlas, "default", 0, 1, &frame) && frame == 1);
    mv_atlas_destroy(atlas);
    char edge[2048];
    memcpy(edge, buffer, length + 1);
    char *offset = strstr(edge, "\"x\":2,\"y\":3");
    assert(offset);
    offset[strlen("\"x\":")] = '4';
    offset[strlen("\"x\":2,\"y\":")] = '4';
    atlas = mv_atlas_parse_aseprite(edge, length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite));
    assert(sprite.trim_x == 4 && sprite.trim_y == 4);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(buffer, "\"trimmed\":true", "\"trimmed\":false");
    invalid_aseprite_change(buffer, "\"x\":2,\"y\":3", "\"x\":5,\"y\":3");
    invalid_aseprite_change(buffer, "\"x\":2,\"y\":3", "\"x\":2,\"y\":5");
    invalid_aseprite_change(buffer, "\"x\":2,\"y\":3", "\"x\":-1,\"y\":3");
    invalid_aseprite_change(buffer, "\"sourceSize\":{\"w\":8", "\"sourceSize\":{\"w\":3");
    file = fopen(argv[5], "rb");
    assert(file);
    length = fread(buffer, 1, sizeof(buffer) - 1, file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    buffer[length] = '\0';
    atlas = mv_atlas_parse_aseprite(buffer, length, 8, 8, &error);
    assert(atlas && !error);
    unsigned repeats = 99;
    assert(mv_atlas_animation_repeats(atlas, "bounce", &repeats) && repeats == 3);
    assert(mv_atlas_animation_repeats(atlas, "single", &repeats) && repeats == 0);
    assert(!mv_atlas_animation_repeats(atlas, "default", &repeats) && repeats == 0);
    assert(!mv_atlas_animation_repeats(atlas, "missing", &repeats) && repeats == 0);
    assert(!mv_atlas_animation_repeats(NULL, "bounce", &repeats));
    assert(!mv_atlas_animation_repeats(atlas, NULL, &repeats));
    assert(!mv_atlas_animation_repeats(atlas, "bounce", NULL));
    MvColor color = {1, 2, 3, 4};
    assert(mv_atlas_animation_color(atlas, "bounce", &color));
    assert(color.r == 0x12 && color.g == 0xab && color.b == 0xcd && color.a == 0x80);
    assert(mv_atlas_animation_color(atlas, "single", &color));
    assert(color.r == 0xab && color.g == 0xcd && color.b == 0xef && color.a == 255);
    assert(!mv_atlas_animation_color(atlas, "default", &color) && color.a == 255);
    assert(!mv_atlas_animation_color(atlas, "missing", &color) && color.r == 0xab);
    assert(!mv_atlas_animation_color(NULL, "bounce", &color));
    assert(!mv_atlas_animation_color(atlas, NULL, &color));
    assert(!mv_atlas_animation_color(atlas, "bounce", NULL));
    assert(mv_atlas_animation_frame(atlas, "bounce", 0, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "bounce", 100, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "bounce", 300, 1, &frame) && frame == 3);
    assert(mv_atlas_animation_frame(atlas, "bounce", 600, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "bounce", 800, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "bounce", UINT64_MAX, 0, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "backbounce", 0, 1, &frame) && frame == 3);
    assert(mv_atlas_animation_frame(atlas, "backbounce", 300, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "backbounce", 500, 1, &frame) && frame == 1);
    assert(mv_atlas_animation_frame(atlas, "backbounce", 600, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "backbounce", 800, 1, &frame) && frame == 3);
    assert(mv_atlas_animation_frame(atlas, "single", 200, 1, &frame) && frame == 2);
    assert(mv_atlas_animation_frame(atlas, "pair", 200, 1, &frame) && frame == 1);
    mv_atlas_destroy(atlas);
    const char *repeat_field = strstr(buffer, "\"repeat\":\"3\"");
    assert(repeat_field);
    char maximum[2048];
    int maximum_length = snprintf(maximum, sizeof(maximum), "%.*s%s%s",
                                  (int)(repeat_field - buffer), buffer,
                                  "\"repeat\":\"65535\"", repeat_field + strlen("\"repeat\":\"3\""));
    assert(maximum_length > 0 && (size_t)maximum_length < sizeof(maximum));
    atlas = mv_atlas_parse_aseprite(maximum, (size_t)maximum_length, 8, 8, &error);
    assert(atlas && mv_atlas_animation_repeats(atlas, "bounce", &repeats) && repeats == 65535);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(buffer, "\"repeat\":\"3\"", "\"repeat\":\"65536\"");
    invalid_aseprite_change(buffer, "\"repeat\":\"3\"", "\"repeat\":\"\"");
    invalid_aseprite_change(buffer, "\"repeat\":\"3\"", "\"repeat\":3");
    invalid_aseprite_change(buffer, "\"repeat\":\"3\"", "\"repeat\":\"1x\"");
    invalid_aseprite_change(buffer, "\"repeat\":\"3\"", "\"repeat\":\"2\",\"repeat\":\"3\"");
    invalid_aseprite_change(buffer, "#12aBcD80", "#12aBcD8z");
    invalid_aseprite_change(buffer, "#12aBcD80", "#123");
    invalid_aseprite_change(buffer, "\"color\":\"#abcdef\"", "\"color\":123");
    invalid_aseprite_change(buffer, "\"color\":\"#abcdef\"", "\"color\":\"#abcdef\",\"color\":\"#abcdef\"");
    file = fopen(argv[6], "rb");
    assert(file);
    length = fread(buffer, 1, sizeof(buffer) - 1, file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    buffer[length] = '\0';
    atlas = mv_atlas_parse_aseprite(buffer, length, 8, 8, &error);
    assert(atlas && !error && mv_atlas_find(atlas, 1, &sprite));
    assert(sprite.pivot_x == 2 && sprite.pivot_y == 3);
    assert(sprite.trim_x == 2 && sprite.trim_y == 3);
    MvColor slice_color = {0};
    assert(mv_atlas_pivot_color(atlas, &slice_color));
    assert(slice_color.r == 0x12 && slice_color.g == 0x34 && slice_color.b == 0x56 && slice_color.a == 0x78);
    assert(!mv_atlas_pivot_color(NULL, &slice_color));
    assert(!mv_atlas_pivot_color(atlas, NULL));
    mv_atlas_destroy(atlas);
    const char *frames_end = strstr(buffer, "\n  ],\n  \"meta\"");
    assert(frames_end);
    char multiple[4096];
    const char *second_frame = ",\n    {\"filename\":\"full\",\"frame\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8},"
        "\"rotated\":false,\"trimmed\":false,\"spriteSourceSize\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8},"
        "\"sourceSize\":{\"w\":8,\"h\":8},\"duration\":200}";
    int multiple_length = snprintf(multiple, sizeof(multiple), "%.*s%s%s",
                                   (int)(frames_end - buffer), buffer, second_frame, frames_end);
    assert(multiple_length > 0 && (size_t)multiple_length < sizeof(multiple));
    atlas = mv_atlas_parse_aseprite(multiple, (size_t)multiple_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite) && sprite.pivot_x == 2 && sprite.pivot_y == 3);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.pivot_x == 4 && sprite.pivot_y == 6);
    assert(mv_atlas_animation_frame(atlas, "default", 120, 1, &frame) && frame == 2);
    mv_atlas_destroy(atlas);
    const char *key_end = strstr(multiple, "}}]}\n");
    assert(key_end);
    char animated[4096];
    const char *next_key = "}},{\"frame\":1,\"bounds\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8},"
        "\"pivot\":{\"x\":1,\"y\":2";
    int animated_length = snprintf(animated, sizeof(animated), "%.*s%s%s",
                                   (int)(key_end - multiple), multiple, next_key, key_end);
    assert(animated_length > 0 && (size_t)animated_length < sizeof(animated));
    atlas = mv_atlas_parse_aseprite(animated, (size_t)animated_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite) && sprite.pivot_x == 2 && sprite.pivot_y == 3);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.pivot_x == 1 && sprite.pivot_y == 2);
    mv_atlas_destroy(atlas);
    const char *second_pivot = strstr(animated, ",\"pivot\":{\"x\":1,\"y\":2}");
    assert(second_pivot);
    char no_pivot[4096];
    int no_pivot_length = snprintf(no_pivot, sizeof(no_pivot), "%.*s%s",
        (int)(second_pivot - animated), animated,
        second_pivot + strlen(",\"pivot\":{\"x\":1,\"y\":2}"));
    assert(no_pivot_length > 0 && (size_t)no_pivot_length < sizeof(no_pivot));
    atlas = mv_atlas_parse_aseprite(no_pivot, (size_t)no_pivot_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite) && sprite.pivot_x == 2 && sprite.pivot_y == 3);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.pivot_x == 0 && sprite.pivot_y == 0);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(no_pivot, "\"frame\":1,\"bounds\"", "\"frame\":1,\"pivot\":null,\"bounds\"");
    const char *second_bounds = strstr(animated, "\"frame\":1,\"bounds\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8}");
    assert(second_bounds);
    char hidden[4096];
    int hidden_length = snprintf(hidden, sizeof(hidden), "%.*s%s%s",
        (int)(second_bounds - animated), animated,
        "\"frame\":1,\"bounds\":{\"x\":0,\"y\":0,\"w\":0,\"h\":0}",
        second_bounds + strlen("\"frame\":1,\"bounds\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8}"));
    assert(hidden_length > 0 && (size_t)hidden_length < sizeof(hidden));
    atlas = mv_atlas_parse_aseprite(hidden, (size_t)hidden_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite) && sprite.pivot_x == 2 && sprite.pivot_y == 3);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.pivot_x == 0 && sprite.pivot_y == 0);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(hidden, "\"w\":0,\"h\":0", "\"w\":-1,\"h\":0");
    invalid_aseprite_change(animated, "\"frame\":1", "\"frame\":0");
    invalid_aseprite_change(animated, "\"frame\":1", "\"frame\":2");
    const char *first_key = strstr(multiple, "\"frame\":0,\"bounds\"");
    assert(first_key);
    char delayed[4096];
    int delayed_length = snprintf(delayed, sizeof(delayed), "%.*s%s%s",
        (int)(first_key - multiple), multiple, "\"frame\":1,\"bounds\"",
        first_key + strlen("\"frame\":0,\"bounds\""));
    assert(delayed_length > 0 && (size_t)delayed_length < sizeof(delayed));
    atlas = mv_atlas_parse_aseprite(delayed, (size_t)delayed_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite));
    assert(sprite.pivot_x == -2 && sprite.pivot_y == -3);
    assert(mv_atlas_find(atlas, 2, &sprite) && sprite.pivot_x == 4 && sprite.pivot_y == 6);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(delayed, "\"frame\":1,\"bounds\"", "\"frame\":2,\"bounds\"");
    invalid_aseprite_change(multiple, "\"sourceSize\":{\"w\":8,\"h\":8}",
                            "\"sourceSize\":{\"w\":7,\"h\":8}");
    invalid_aseprite_change(buffer, "\"name\":\"pivot\"", "\"name\":\"other\"");
    invalid_aseprite_change(buffer, "\"frame\":0", "\"frame\":0,\"frame\":0");
    invalid_aseprite_change(buffer, "\"pivot\":{\"x\":4,\"y\":6}", "\"pivot\":{\"x\":4,\"y\":6,\"z\":0}");
    invalid_aseprite_change(buffer, "\"frame\":0", "\"frame\":1");
    invalid_aseprite_change(buffer, "\"pivot\":{\"x\":4", "\"pivot\":{\"x\":-2147483648");
    const char *bounds = strstr(buffer, "\"bounds\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8}");
    assert(bounds);
    char partial[4096];
    int partial_length = snprintf(partial, sizeof(partial), "%.*s%s%s",
        (int)(bounds - buffer), buffer, "\"bounds\":{\"x\":1,\"y\":2,\"w\":6,\"h\":5}",
        bounds + strlen("\"bounds\":{\"x\":0,\"y\":0,\"w\":8,\"h\":8}"));
    assert(partial_length > 0 && (size_t)partial_length < sizeof(partial));
    atlas = mv_atlas_parse_aseprite(partial, (size_t)partial_length, 8, 8, &error);
    assert(atlas && mv_atlas_find(atlas, 1, &sprite) && sprite.pivot_x == 3 && sprite.pivot_y == 5);
    mv_atlas_destroy(atlas);
    invalid_aseprite_change(partial, "\"x\":1,\"y\":2,\"w\":6", "\"x\":3,\"y\":2,\"w\":6");

    invalid_aseprite_change(buffer, "#12345678", "#oops");
    invalid_aseprite_change(buffer, "\"frame\":0", "\"center\":{},\"frame\":0");
    invalid_aseprite_change(buffer, "\"image\":\"red.png\"", "\"app\":\"bad\nvalue\",\"image\":\"red.png\"");
    invalid_aseprite_change(buffer, "\"name\":\"pivot\"", "\"name\":\"pivot\\u0000extra\"");
    invalid_aseprite_change(buffer, "\"keys\"", "\"keys\\u0000extra\"");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\\u0000hidden\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}]}");
    /* A literal backslash and u0000 are not an embedded zero byte. */
    const char *literal = "{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a\\\\u0000.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}]}";
    atlas = mv_atlas_parse(literal, strlen(literal), 8, 8, &error);
    assert(atlas);
    mv_atlas_destroy(atlas);
    const char *image_field = strstr(buffer, "\"image\":\"red.png\"");
    assert(image_field);
    char literal_aseprite[4096];
    int literal_length = snprintf(literal_aseprite, sizeof(literal_aseprite), "%.*s%s%s",
        (int)(image_field - buffer), buffer, "\"image\":\"a\\\\u0000.png\"",
        image_field + strlen("\"image\":\"red.png\""));
    assert(literal_length > 0 && (size_t)literal_length < sizeof(literal_aseprite));
    atlas = mv_atlas_parse_aseprite(literal_aseprite, (size_t)literal_length, 8, 8, &error);
    assert(atlas);
    mv_atlas_destroy(atlas);
    invalid("{}");
    invalid("{\"schema\":01,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[]}");
    invalid("{\"schema\":1.,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[]}");
    invalid("{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[]}");
    invalid("{\"schema\":1,\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":7,\"y\":0,\"width\":2,\"height\":1}]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1,\"rotation\":0}]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1},{\"id\":1,\"name\":\"b\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1},{\"id\":2,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a\\u0000.png\",\"sprites\":[]}");
    invalid("{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}],\"animations\":[{\"name\":\"run\",\"frames\":[{\"sprite\":2,\"duration_ms\":10}]}]}");
    invalid("{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}],\"animations\":[{\"name\":\"run\",\"frames\":[{\"sprite\":1,\"duration_ms\":0}]}]}");
    invalid("{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}],\"animations\":[{\"name\":\"run\",\"frames\":[{\"sprite\":1,\"duration_ms\":1}]},{\"name\":\"run\",\"frames\":[{\"sprite\":1,\"duration_ms\":1}]}]}");
    invalid("{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}],\"animations\":[]}");
    invalid("{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"a.png\",\"sprites\":[{\"id\":1,\"name\":\"a\",\"x\":0,\"y\":0,\"width\":1,\"height\":1}],\"animations\":[{\"name\":\"run\",\"frames\":[{\"sprite\":1,\"duration_ms\":1,\"extra\":0}]}]}");
    assert(!mv_atlas_parse(buffer, length, 3, 8, &error));
    assert(!mv_atlas_parse(buffer, 1024 * 1024 + 1, 8, 8, &error));
    return 0;
}
