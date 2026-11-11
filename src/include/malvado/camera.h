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

#include "malvado/malvado.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Caller-owned values, independent of Lua and graphics. A zero viewport uses
 * the full window; an explicit viewport has positive width and height. */
typedef struct { int x, y, width, height; } MvCameraViewport;
typedef struct { double x, y; MvCameraViewport viewport; } MvCamera;

/* Context-owned camera identities. Main is ID 1 and cannot be closed. Explicit
 * IDs are never reused. Values own no platform resource. Context shutdown frees
 * all cameras. Closing an explicit camera is idempotent; selected processes hide
 * until reassigned. References do not retain a camera or process. */
typedef struct { MvRuntimeId runtime; uint64_t id; } MvCameraRef;
MvResult mv_camera_main(MvContext *ctx, MvCameraRef *out);
MvResult mv_camera_create(MvContext *ctx, MvCamera value, MvCameraRef *out);
MvResult mv_camera_get(MvContext *ctx, MvCameraRef camera, MvCamera *out);
MvResult mv_camera_put(MvContext *ctx, MvCameraRef camera, MvCamera value);
MvResult mv_camera_close(MvContext *ctx, MvCameraRef camera);
/* New processes use main. NULL selects main; an all-zero reference selects
 * screen space; otherwise require a live same-context camera. Get copies the
 * selection, even after explicit close. Resolve copies effective camera values
 * or returns MV_NOT_FOUND for a closed selected camera. Error outputs/state
 * are unchanged. Foreign/NULL/invalid values return MV_INVALID_ARGUMENT; ended
 * processes return MV_NOT_FOUND. Owner-thread only. Only create allocates. */
MvResult mv_process_camera_set(MvContext *ctx, MvProcessRef process, const MvCameraRef *camera);
MvResult mv_process_camera_get(MvContext *ctx, MvProcessRef process, MvCameraRef *out);
MvResult mv_process_camera_resolve(MvContext *ctx, MvProcessRef process, MvCamera *out);

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

/* Center a target origin in the viewport, or in the supplied logical window.
 * Call from the game's existing update flow. Invalid inputs leave state intact. */
int mv_camera_center(MvCamera *camera, double target_x, double target_y,
                     int window_width, int window_height);

/* Update a caller-owned follow target after the native tick and removals.
 * target is a context-bound process reference, or all-zero for no target.
 * A live target is centered from native x/y, even when sleeping or frozen.
 * An ended target is cleared and the final camera position is preserved.
 * Return MV_OK, or MV_INVALID_ARGUMENT for NULL inputs, foreign/malformed
 * references or invalid camera/window/target geometry. Failure preserves both
 * camera and target. The context must be live on its owner thread. Nothing is
 * allocated, retained, scheduled or destroyed; copied follow references do not
 * keep processes or resources alive. An inactive target is a no-op. */
MvResult mv_camera_follow_step(MvCamera *camera, MvContext *ctx,
                               MvProcessRef *target, int window_width, int window_height);

/* Intersect viewport, optional process region and the screen. Return 1 and a
 * visible rectangle, 0 and a zero rectangle for an empty intersection, or -1
 * on invalid input without changing output. Screen dimensions must be positive.
 * A supplied region must have positive dimensions. No projection is applied. */
int mv_camera_clip(const MvCamera *camera, const MvCameraViewport *region,
                   int screen_width, int screen_height, MvCameraViewport *out);

#ifdef __cplusplus
}
#endif
#endif
