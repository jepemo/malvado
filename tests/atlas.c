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

int
main(int argc, char **argv)
{
    assert(argc == 3);
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
    assert(!mv_atlas_find(atlas, 8, &sprite));
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
