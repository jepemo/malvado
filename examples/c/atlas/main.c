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
#include <string.h>
#include "malvado/atlas.h"

int
main(void)
{
    const char *json =
        "{\"schema\":2,\"kind\":\"graphics-library\",\"image\":\"ship.png\","
        "\"sprites\":[{\"id\":7,\"name\":\"ship\",\"x\":0,\"y\":0,"
        "\"width\":16,\"height\":16}],\"animations\":[{\"name\":\"idle\","
        "\"frames\":[{\"sprite\":7,\"duration_ms\":100}]}]}";
    const char *error = NULL;
    MvAtlas *atlas = mv_atlas_parse(json, strlen(json), 16, 16, &error);
    if (!atlas) {
        fprintf(stderr, "Cannot parse atlas: %s\n", error ? error : "unknown error");
        return 1;
    }

    MvAtlasSprite sprite;
    int frame;
    int ok = mv_atlas_find_name(atlas, "ship", &sprite) &&
             mv_atlas_animation_frame(atlas, "idle", 100, 1, &frame);
    if (ok)
        printf("%s: %s, sprite %d, frame %d\n", mv_atlas_image(atlas), sprite.name, sprite.id, frame);
    mv_atlas_destroy(atlas);
    return ok ? 0 : 1;
}
