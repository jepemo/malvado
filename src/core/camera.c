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

#include "malvado/camera.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

static int
coordinate_valid(double value)
{
    return isfinite(value) && fabs(value) <= FLT_MAX / 64;
}

static int
camera_valid(const MvCamera *camera)
{
    if (!camera || !coordinate_valid(camera->x) || !coordinate_valid(camera->y))
        return 0;
    const MvCameraViewport *view = &camera->viewport;
    return (view->width == 0 && view->height == 0 && view->x == 0 && view->y == 0) ||
           (view->width > 0 && view->height > 0);
}

int
mv_camera_init(MvCamera *camera, double x, double y, const MvCameraViewport *viewport)
{
    MvCamera value = {x, y, {0}};
    if (viewport) {
        if (viewport->width <= 0 || viewport->height <= 0)
            return 0;
        value.viewport = *viewport;
    }
    if (!camera || !camera_valid(&value))
        return 0;
    *camera = value;
    return 1;
}

int
mv_camera_set(MvCamera *camera, double x, double y)
{
    if (!camera_valid(camera) || !coordinate_valid(x) || !coordinate_valid(y))
        return 0;
    camera->x = x;
    camera->y = y;
    return 1;
}

int
mv_camera_move(MvCamera *camera, double dx, double dy)
{
    if (!camera_valid(camera) || !coordinate_valid(dx) || !coordinate_valid(dy))
        return 0;
    return mv_camera_set(camera, camera->x + dx, camera->y + dy);
}

int
mv_camera_project(const MvCamera *camera, double world_x, double world_y,
                  double scroll_x, double scroll_y, double *screen_x, double *screen_y)
{
    if (!camera_valid(camera) || !screen_x || !screen_y || screen_x == screen_y ||
        !coordinate_valid(world_x) || !coordinate_valid(world_y) ||
        !isfinite(scroll_x) || !isfinite(scroll_y) ||
        fabs(scroll_x) > FLT_MAX / 16 || fabs(scroll_y) > FLT_MAX / 16)
        return 0;
    double x = world_x - camera->x - scroll_x + camera->viewport.x;
    double y = world_y - camera->y - scroll_y + camera->viewport.y;
    if (!isfinite(x) || !isfinite(y) || fabs(x) > FLT_MAX / 16 || fabs(y) > FLT_MAX / 16)
        return 0;
    *screen_x = x;
    *screen_y = y;
    return 1;
}


int
mv_camera_center(MvCamera *camera, double target_x, double target_y,
                 int window_width, int window_height)
{
    if (!camera_valid(camera) || !coordinate_valid(target_x) || !coordinate_valid(target_y) ||
        window_width <= 0 || window_height <= 0)
        return 0;
    int width = camera->viewport.width ? camera->viewport.width : window_width;
    int height = camera->viewport.height ? camera->viewport.height : window_height;
    return mv_camera_set(camera, target_x - width / 2.0, target_y - height / 2.0);
}


MvResult
mv_camera_follow_step(MvCamera *camera, MvContext *ctx, MvProcessRef *target,
                      int window_width, int window_height)
{
    if (!camera_valid(camera) || !ctx || !target || window_width <= 0 || window_height <= 0)
        return MV_INVALID_ARGUMENT;
    if (!target->id) {
        if (target->runtime.high || target->runtime.low || target->runtime.sequence)
            return MV_INVALID_ARGUMENT;
        return MV_OK;
    }
    double x, y;
    MvResult result = mv_process_position(ctx, *target, &x, &y);
    if (result == MV_NOT_FOUND) {
        *target = (MvProcessRef){0};
        return MV_OK;
    }
    if (result != MV_OK)
        return result;
    if (!mv_camera_center(camera, x, y, window_width, window_height))
        return MV_INVALID_ARGUMENT;
    return MV_OK;
}

int
mv_camera_clip(const MvCamera *camera, const MvCameraViewport *region,
               int screen_width, int screen_height, MvCameraViewport *out)
{
    if (!camera_valid(camera) || !out || screen_width <= 0 || screen_height <= 0 ||
        (region && (region->width <= 0 || region->height <= 0)))
        return -1;
    int64_t left = 0, top = 0, right = screen_width, bottom = screen_height;
    const MvCameraViewport *clips[] = {&camera->viewport, region};
    for (size_t i = 0; i < 2; ++i) {
        const MvCameraViewport *clip = clips[i];
        if (!clip || !clip->width) continue;
        if (left < clip->x) left = clip->x;
        if (top < clip->y) top = clip->y;
        int64_t clip_right = (int64_t)clip->x + clip->width;
        int64_t clip_bottom = (int64_t)clip->y + clip->height;
        if (right > clip_right) right = clip_right;
        if (bottom > clip_bottom) bottom = clip_bottom;
    }
    *out = (MvCameraViewport){0};
    if (right <= left || bottom <= top) return 0;
    *out = (MvCameraViewport){(int)left, (int)top, (int)(right - left), (int)(bottom - top)};
    return 1;
}
