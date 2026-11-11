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
#include "malvado/atlas.h"
#include <assert.h>
#include <string.h>

static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
int main(void)
{
    const char *json = "{\"schema\":1,\"kind\":\"graphics-library\",\"image\":\"red.png\","
                       "\"sprites\":[{\"id\":7,\"name\":\"red\",\"x\":0,\"y\":0,\"width\":4,\"height\":8}]}";
    MvAtlas *atlas = mv_atlas_parse(json, strlen(json), 8, 8, NULL);
    assert(atlas);
    MvContext *ctx = mv_context_create(60, 4), *other = mv_context_create(60, 4);
    assert(ctx && other);
    MvProcessId id;
    MvProcessRef owner;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &owner) == MV_OK);
    MvResource *resource = NULL;
    assert(mv_resource_adopt_atlas(NULL, NULL, atlas, &resource) == MV_INVALID_ARGUMENT && !resource);
    assert(mv_resource_adopt_atlas(ctx, NULL, NULL, &resource) == MV_INVALID_ARGUMENT && !resource);
    assert(mv_resource_adopt_atlas(ctx, NULL, atlas, NULL) == MV_INVALID_ARGUMENT);
    assert(mv_resource_adopt_atlas(other, &owner, atlas, &resource) == MV_INVALID_ARGUMENT && !resource);
    assert(mv_atlas_count(atlas) == 1);
    assert(mv_resource_adopt_atlas(ctx, &owner, atlas, &resource) == MV_OK);
    assert(mv_resource_retain(resource) == MV_OK);
    assert(mv_resource_data(resource) == atlas);
    assert(mv_resource_transfer(resource, NULL) == MV_OK);
    int killed;
    assert(mv_process_kill(ctx, owner, &killed) == MV_OK && killed);
    assert(mv_process_tick(ctx) == MV_OK);
    MvAtlas *failed = mv_atlas_parse(json, strlen(json), 8, 8, NULL);
    MvResource *out = resource;
    assert(failed && mv_resource_adopt_atlas(ctx, &owner, failed, &out) == MV_NOT_FOUND && out == resource);
    mv_atlas_destroy(failed);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    MvAtlasSprite sprite;
    assert(mv_atlas_find(atlas, 7, &sprite) && !strcmp(sprite.name, "red"));
    assert(mv_resource_release(resource) == MV_OK);
    /* No independent reference: context cleanup destroys metadata directly. */
    ctx = mv_context_create(60, 4);
    atlas = mv_atlas_parse(json, strlen(json), 8, 8, NULL);
    assert(ctx && atlas && mv_resource_adopt_atlas(ctx, NULL, atlas, &resource) == MV_OK);
    mv_context_destroy(ctx);
    return 0;
}
