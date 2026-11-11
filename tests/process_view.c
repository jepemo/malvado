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
#include "malvado/camera.h"
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
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
    MvContext *ctx = mv_context_create(60,4), *other = mv_context_create(60,4);
    assert(ctx && other);
    MvProcessId id;
    MvProcessRef ref;
    assert(mv_process_spawn(ctx,0,0,idle,NULL,NULL,&id) == MV_OK);
    assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
    MvCameraViewport region = {10,20,40,30}, out = region, before = out;
    int present = 99;
    assert(mv_process_region_get(ctx,ref,&out,&present) == MV_OK && !present);
    assert(memcmp(&out,&before,sizeof(out)) == 0);
    float x = 123, y = 456;
    assert(mv_process_scroll_get(ctx,ref,&x,&y) == MV_OK && x == 0 && y == 0);
    assert(mv_process_region_set(ctx,ref,&region) == MV_OK);
    region.width = 1;
    assert(mv_process_region_get(ctx,ref,&out,&present) == MV_OK && present && out.width == 40);
    assert(mv_process_scroll_set(ctx,ref,3,4) == MV_OK);
    MvCameraRef main;
    MvCamera camera;
    assert(mv_camera_main(ctx,&main) == MV_OK);
    assert(mv_camera_init(&camera,100,50,NULL));
    assert(mv_camera_put(ctx,main,camera) == MV_OK);
    double dx, dy;
    assert(mv_process_view_offset(ctx,ref,&dx,&dy) == MV_OK && dx == -103 && dy == -54);
    MvCameraViewport host = {20,25,10,10};
    assert(mv_process_view_clip(ctx,ref,&host,100,80,&out) == MV_OK && out.x == 20 && out.y == 25 && out.width == 10 && out.height == 10);
    region = (MvCameraViewport){INT_MAX,INT_MIN,INT_MAX,INT_MAX};
    assert(mv_process_region_set(ctx,ref,&region) == MV_OK);
    assert(mv_process_view_clip(ctx,ref,NULL,100,80,&out) == MV_OK && !out.width && !out.height);
    before = out;
    present = 99;
    region.width = 0;
    assert(mv_process_region_set(ctx,ref,&region) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_set(other,ref,NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_get(other,ref,&out,&present) == MV_INVALID_ARGUMENT && present == 99);
    assert(mv_process_region_get(ctx,ref,&out,&out.width) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_get(ctx,ref,NULL,&present) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_get(ctx,ref,&out,NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_get(NULL,ref,&out,&present) == MV_INVALID_ARGUMENT);
    assert(mv_process_scroll_get(ctx,ref,&x,&x) == MV_INVALID_ARGUMENT);
    assert(mv_process_scroll_get(ctx,ref,NULL,&y) == MV_INVALID_ARGUMENT);
    assert(mv_process_scroll_get(ctx,ref,&x,NULL) == MV_INVALID_ARGUMENT);
    assert(mv_process_scroll_get(other,ref,&x,&y) == MV_INVALID_ARGUMENT);
    assert(mv_process_scroll_set(other,ref,1,2) == MV_INVALID_ARGUMENT);
    float invalid[] = {NAN,INFINITY,-INFINITY,FLT_MAX};
    for (unsigned i=0; i<4; ++i) {
        assert(mv_process_scroll_set(ctx,ref,invalid[i],0) == MV_INVALID_ARGUMENT);
        assert(mv_process_scroll_set(ctx,ref,0,invalid[i]) == MV_INVALID_ARGUMENT);
    }
    assert(mv_process_scroll_get(ctx,ref,&x,&y) == MV_OK && x == 3 && y == 4);
    assert(mv_process_view_clip(ctx,ref,NULL,0,80,&out) == MV_INVALID_ARGUMENT);
    host.width = 0;
    assert(mv_process_view_clip(ctx,ref,&host,100,80,&out) == MV_INVALID_ARGUMENT);
    assert(mv_process_view_clip(ctx,ref,NULL,100,80,NULL) == MV_INVALID_ARGUMENT);
    assert(memcmp(&out,&before,sizeof(out)) == 0);
    assert(mv_process_view_offset(ctx,ref,&dx,&dx) == MV_INVALID_ARGUMENT);
    assert(mv_process_view_offset(ctx,ref,NULL,&dy) == MV_INVALID_ARGUMENT);
    assert(mv_process_view_offset(other,ref,&dx,&dy) == MV_INVALID_ARGUMENT);
    assert(mv_process_region_set(ctx,ref,NULL) == MV_OK);
    assert(mv_process_view_clip(ctx,ref,NULL,100,80,&out) == MV_OK && out.width == 100 && out.height == 80);
    assert(mv_process_scroll_set(ctx,ref,FLT_MAX/16,-FLT_MAX/16) == MV_OK);
    assert(mv_process_scroll_get(ctx,ref,&x,&y) == MV_OK && x == FLT_MAX/16);
    assert(mv_process_scroll_set(ctx,ref,3,4) == MV_OK);
    MvCameraRef explicit;
    assert(mv_camera_create(ctx,camera,&explicit) == MV_OK);
    assert(mv_process_camera_set(ctx,ref,&explicit) == MV_OK);
    assert(mv_camera_close(ctx,explicit) == MV_OK);
    before = out; dx = 123; dy = 456;
    assert(mv_process_view_clip(ctx,ref,NULL,100,80,&out) == MV_NOT_FOUND);
    assert(mv_process_view_offset(ctx,ref,&dx,&dy) == MV_NOT_FOUND && dx == 123 && dy == 456);
    assert(memcmp(&out,&before,sizeof(out)) == 0);
    int killed;
    assert(mv_process_kill(ctx,ref,&killed) == MV_OK && killed);
    assert(mv_process_region_set(ctx,ref,NULL) == MV_NOT_FOUND);
    assert(mv_process_region_get(ctx,ref,&out,&present) == MV_NOT_FOUND);
    assert(mv_process_scroll_set(ctx,ref,0,0) == MV_NOT_FOUND);
    assert(mv_process_scroll_get(ctx,ref,&x,&y) == MV_NOT_FOUND);
    mv_context_destroy(other);
    mv_context_destroy(ctx);
    return 0;
}
