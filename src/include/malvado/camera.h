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

#ifndef MALVADO_CAMERA_H
#define MALVADO_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Caller-owned values, independent of Lua and graphics. A zero viewport uses
 * the full window; an explicit viewport has positive width and height. */
typedef struct { int x, y, width, height; } MvCameraViewport;
typedef struct { double x, y; MvCameraViewport viewport; } MvCamera;

/* Initialize a camera. NULL viewport selects the full window. Coordinates must
 * be finite with magnitude <= FLT_MAX / 64; explicit viewport dimensions must
 * be positive. Return 1 on success, 0 without changing outputs on failure.
 * No allocation, retained pointers, or cleanup is required. */
int mv_camera_init(MvCamera *camera, double x, double y, const MvCameraViewport *viewport);
int mv_camera_set(MvCamera *camera, double x, double y);
int mv_camera_move(MvCamera *camera, double dx, double dy);
/* Project world - camera - local scroll + viewport origin. The output pointers
 * must be distinct. All coordinates obey the same numeric limit; results must
 * fit FLT_MAX / 16. Clipping is a separate screen-space operation. */
int mv_camera_project(const MvCamera *camera, double world_x, double world_y,
                      double scroll_x, double scroll_y, double *screen_x, double *screen_y);

#ifdef __cplusplus
}
#endif
#endif
