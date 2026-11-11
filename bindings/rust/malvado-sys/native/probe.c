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
#include "malvado/malvado.h"
#include "malvado/collision.h"
#include "malvado/camera.h"
#include "malvado/animation.h"
#include "malvado/graphic.h"
#include <stdio.h>
#include <string.h>
_Static_assert(_Generic(&mv_abi_version, uint32_t (*)(void): 1, default: 0), "mv_abi_version signature");
_Static_assert(_Generic(&mv_version_string, const char * (*)(void): 1, default: 0), "mv_version_string signature");
_Static_assert(_Generic(&mv_context_create, MvContext * (*)(double, unsigned): 1, default: 0), "mv_context_create signature");
_Static_assert(_Generic(&mv_context_destroy, void (*)(MvContext *): 1, default: 0), "mv_context_destroy signature");
_Static_assert(_Generic(&mv_context_runtime_id, MvResult (*)(MvContext *, MvRuntimeId *): 1, default: 0), "mv_context_runtime_id signature");
_Static_assert(_Generic(&mv_context_advance, MvResult (*)(MvContext *, double, unsigned *): 1, default: 0), "mv_context_advance signature");
_Static_assert(_Generic(&mv_context_ticks, uint64_t (*)(const MvContext *): 1, default: 0), "mv_context_ticks signature");
_Static_assert(_Generic(&mv_context_dropped_seconds, double (*)(const MvContext *): 1, default: 0), "mv_context_dropped_seconds signature");
_Static_assert(_Generic(&mv_cadence_step, int (*)(MvCadence *, unsigned): 1, default: 0), "mv_cadence_step signature");
_Static_assert(_Generic(&mv_process_spawn, MvResult (*)(MvContext *, uint64_t, int64_t, MvProcessStep, MvProcessDestroy, void *, uint64_t *): 1, default: 0), "mv_process_spawn signature");
_Static_assert(_Generic(&mv_process_spawn_related, MvResult (*)(MvContext *, uint64_t, uint64_t, int64_t, MvProcessStep, MvProcessDestroy, void *, uint64_t *): 1, default: 0), "mv_process_spawn_related signature");
_Static_assert(_Generic(&mv_process_tick, MvResult (*)(MvContext *): 1, default: 0), "mv_process_tick signature");
_Static_assert(_Generic(&mv_process_count, size_t (*)(const MvContext *): 1, default: 0), "mv_process_count signature");
_Static_assert(_Generic(&mv_process_signal, MvResult (*)(MvContext *, uint64_t, MvSignal, int): 1, default: 0), "mv_process_signal signature");
_Static_assert(_Generic(&mv_process_kill, MvResult (*)(MvContext *, MvProcessRef, int *): 1, default: 0), "mv_process_kill signature");
_Static_assert(_Generic(&mv_process_ref_create, MvResult (*)(MvContext *, uint64_t, MvProcessRef *): 1, default: 0), "mv_process_ref_create signature");
_Static_assert(_Generic(&mv_process_ref_resolve, MvResult (*)(MvContext *, MvProcessRef, uint64_t *): 1, default: 0), "mv_process_ref_resolve signature");
_Static_assert(_Generic(&mv_process_handle_create, MvResult (*)(MvContext *, MvProcessRef, MvProcessHandle * *): 1, default: 0), "mv_process_handle_create signature");
_Static_assert(_Generic(&mv_process_handle_snapshot, MvResult (*)(const MvProcessHandle *, MvProcessHandleInfo *): 1, default: 0), "mv_process_handle_snapshot signature");
_Static_assert(_Generic(&mv_process_handle_destroy, void (*)(MvProcessHandle *): 1, default: 0), "mv_process_handle_destroy signature");
_Static_assert(_Generic(&mv_process_info, MvResult (*)(MvContext *, MvProcessRef, MvProcessInfo *): 1, default: 0), "mv_process_info signature");
_Static_assert(_Generic(&mv_process_set_priority, MvResult (*)(MvContext *, uint64_t, int64_t): 1, default: 0), "mv_process_set_priority signature");
_Static_assert(_Generic(&mv_process_set_type, MvResult (*)(MvContext *, uint64_t, uint64_t): 1, default: 0), "mv_process_set_type signature");
_Static_assert(_Generic(&mv_resource_create, MvResult (*)(MvContext *, const MvProcessRef *, void *, MvResourceDestroy, MvResource * *): 1, default: 0), "mv_resource_create signature");
_Static_assert(_Generic(&mv_resource_transfer, MvResult (*)(MvResource *, const MvProcessRef *): 1, default: 0), "mv_resource_transfer signature");
_Static_assert(_Generic(&mv_resource_retain, MvResult (*)(MvResource *): 1, default: 0), "mv_resource_retain signature");
_Static_assert(_Generic(&mv_resource_release, MvResult (*)(MvResource *): 1, default: 0), "mv_resource_release signature");
_Static_assert(_Generic(&mv_resource_data, void * (*)(const MvResource *): 1, default: 0), "mv_resource_data signature");
_Static_assert(_Generic(&mv_process_find, MvResult (*)(MvContext *, uint64_t, MvProcessInfo *, size_t, size_t *): 1, default: 0), "mv_process_find signature");
_Static_assert(_Generic(&mv_process_children, MvResult (*)(MvContext *, uint64_t, uint64_t, MvProcessInfo *, size_t, size_t *): 1, default: 0), "mv_process_children signature");
_Static_assert(_Generic(&mv_process_descendants, MvResult (*)(MvContext *, uint64_t, uint64_t, MvProcessInfo *, size_t, size_t *): 1, default: 0), "mv_process_descendants signature");
_Static_assert(_Generic(&mv_process_creation_children, MvResult (*)(MvContext *, uint64_t, uint64_t, MvProcessInfo *, size_t, size_t *): 1, default: 0), "mv_process_creation_children signature");
_Static_assert(_Generic(&mv_process_creation_descendants, MvResult (*)(MvContext *, uint64_t, uint64_t, MvProcessInfo *, size_t, size_t *): 1, default: 0), "mv_process_creation_descendants signature");
_Static_assert(_Generic(&mv_process_lineage_snapshot, MvResult (*)(MvContext *, MvLineageInfo *, size_t, size_t *): 1, default: 0), "lineage signature");
_Static_assert(_Generic(&mv_process_signal_creation_tree, MvResult (*)(MvContext *, uint64_t, MvSignal): 1, default: 0), "creation tree signal signature");
_Static_assert(_Generic(&mv_rect_overlaps, int (*)(MvRect, MvRect): 1, default: 0), "mv_rect_overlaps signature");
_Static_assert(_Generic(&mv_rect_contains, int (*)(MvRect, double, double): 1, default: 0), "mv_rect_contains signature");
_Static_assert(_Generic(&mv_collision_overlap, int (*)(MvCollisionShape, MvCollisionTransform, MvCollisionShape, MvCollisionTransform): 1, default: 0), "mv_collision_overlap signature");
_Static_assert(_Generic(&mv_process_collider_set, MvResult (*)(MvContext *, MvProcessRef, MvCollisionShape, MvCollisionTransform): 1, default: 0), "mv_process_collider_set signature");
_Static_assert(_Generic(&mv_process_collider_clear, MvResult (*)(MvContext *, MvProcessRef): 1, default: 0), "mv_process_collider_clear signature");
_Static_assert(_Generic(&mv_process_collider_test, MvResult (*)(MvContext *, MvProcessRef, MvProcessRef, int *): 1, default: 0), "mv_process_collider_test signature");
_Static_assert(_Generic(&mv_process_collider_first, MvResult (*)(MvContext *, MvProcessRef, uint64_t, MvProcessRef *): 1, default: 0), "mv_process_collider_first signature");
_Static_assert(_Generic(&mv_process_collider_find, MvResult (*)(MvContext *, MvProcessRef, uint64_t, MvProcessRef *, size_t, size_t *): 1, default: 0), "mv_process_collider_find signature");
_Static_assert(_Generic(&mv_collision_world_create, MvCollisionWorld * (*)(void): 1, default: 0), "mv_collision_world_create signature");
_Static_assert(_Generic(&mv_collision_world_destroy, void (*)(MvCollisionWorld *): 1, default: 0), "mv_collision_world_destroy signature");
_Static_assert(_Generic(&mv_collision_world_add_rect, int (*)(MvCollisionWorld *, MvRect, size_t *): 1, default: 0), "mv_collision_world_add_rect signature");
_Static_assert(_Generic(&mv_collision_world_query, int (*)(const MvCollisionWorld *, MvCollisionShape, MvCollisionTransform, size_t *): 1, default: 0), "mv_collision_world_query signature");
_Static_assert(_Generic(&mv_collision_world_sweep, int (*)(const MvCollisionWorld *, MvCollisionShape, MvCollisionTransform, double, double, MvCollisionSweep *): 1, default: 0), "mv_collision_world_sweep signature");
_Static_assert(_Generic(&mv_process_world_query, MvResult (*)(MvContext *, MvProcessRef, const MvCollisionWorld *, const MvCollisionShape *, size_t *): 1, default: 0), "mv_process_world_query signature");
_Static_assert(_Generic(&mv_process_world_move, MvResult (*)(MvContext *, MvProcessRef, const MvCollisionWorld *, const MvCollisionShape *, double, double, MvCollisionSweep *): 1, default: 0), "mv_process_world_move signature");
_Static_assert(_Generic(&mv_collision_world_move, int (*)(const MvCollisionWorld *, MvCollisionShape, MvCollisionTransform *, double, double, MvCollisionSweep *): 1, default: 0), "mv_collision_world_move signature");
_Static_assert(_Generic(&mv_camera_init, int (*)(MvCamera *, double, double, const MvCameraViewport *): 1, default: 0), "mv_camera_init signature");
_Static_assert(_Generic(&mv_camera_set, int (*)(MvCamera *, double, double): 1, default: 0), "mv_camera_set signature");
_Static_assert(_Generic(&mv_camera_move, int (*)(MvCamera *, double, double): 1, default: 0), "mv_camera_move signature");
_Static_assert(_Generic(&mv_camera_project, int (*)(const MvCamera *, double, double, double, double, double *, double *): 1, default: 0), "mv_camera_project signature");
_Static_assert(_Generic(&mv_camera_center, int (*)(MvCamera *, double, double, int, int): 1, default: 0), "mv_camera_center signature");
_Static_assert(_Generic(&mv_camera_clip, int (*)(const MvCamera *, const MvCameraViewport *, int, int, MvCameraViewport *): 1, default: 0), "mv_camera_clip signature");
_Static_assert(_Generic(&mv_animation_create, MvAnimation * (*)(const MvAnimationFrame *, size_t, int): 1, default: 0), "mv_animation_create signature");
_Static_assert(_Generic(&mv_animation_destroy, void (*)(MvAnimation *): 1, default: 0), "mv_animation_destroy signature");
_Static_assert(_Generic(&mv_animation_sample, int (*)(const MvAnimation *, uint64_t, MvAnimationSample *): 1, default: 0), "mv_animation_sample signature");
_Static_assert(_Generic(&mv_animation_advance, int (*)(const MvAnimation *, MvAnimationState *, unsigned): 1, default: 0), "mv_animation_advance signature");
_Static_assert(_Generic(&mv_graphic_rect, int (*)(double, double, int, MvCollisionShape *): 1, default: 0), "mv_graphic_rect signature");
_Static_assert(_Generic(&mv_graphic_circle, int (*)(double, MvCollisionShape *): 1, default: 0), "mv_graphic_circle signature");
_Static_assert(_Generic(&mv_process_position, MvResult (*)(MvContext *, MvProcessRef, double *, double *): 1, default: 0), "mv_process_position signature");
_Static_assert(_Generic(&mv_process_set_position, MvResult (*)(MvContext *, MvProcessRef, double, double): 1, default: 0), "mv_process_set_position signature");
_Static_assert(_Generic(&mv_process_angle, MvResult (*)(MvContext *, MvProcessRef, double *): 1, default: 0), "mv_process_angle signature");
_Static_assert(_Generic(&mv_process_set_angle, MvResult (*)(MvContext *, MvProcessRef, double): 1, default: 0), "mv_process_set_angle signature");
_Static_assert(_Generic(&mv_process_scale, MvResult (*)(MvContext *, MvProcessRef, double *): 1, default: 0), "mv_process_scale signature");
_Static_assert(_Generic(&mv_process_set_scale, MvResult (*)(MvContext *, MvProcessRef, double): 1, default: 0), "mv_process_set_scale signature");
_Static_assert(_Generic(&mv_process_z, MvResult (*)(MvContext *, MvProcessRef, double *): 1, default: 0), "mv_process_z signature");
_Static_assert(_Generic(&mv_process_set_z, MvResult (*)(MvContext *, MvProcessRef, double): 1, default: 0), "mv_process_set_z signature");
_Static_assert(_Generic(&mv_process_draw_order, MvResult (*)(MvContext *, MvProcessDrawInfo *, size_t, size_t *): 1, default: 0), "mv_process_draw_order signature");
int main(void) {
    if (mv_abi_version() != 1) { fputs("incompatible Malvado native ABI revision\n", stderr); return 1; }
    if (strcmp(mv_version_string(), EXPECTED_VERSION)) { fputs("incompatible Malvado native release\n", stderr); return 1; }
    printf("%zu %zu ", sizeof(MvCadence), _Alignof(MvCadence));
    printf("%zu ", offsetof(MvCadence, phase));
    printf("%zu %zu ", sizeof(MvRuntimeId), _Alignof(MvRuntimeId));
    printf("%zu ", offsetof(MvRuntimeId, high));
    printf("%zu ", offsetof(MvRuntimeId, low));
    printf("%zu ", offsetof(MvRuntimeId, sequence));
    printf("%zu %zu ", sizeof(MvProcessRef), _Alignof(MvProcessRef));
    printf("%zu ", offsetof(MvProcessRef, runtime));
    printf("%zu ", offsetof(MvProcessRef, id));
    printf("%zu %zu ", sizeof(MvProcessInfo), _Alignof(MvProcessInfo));
    printf("%zu ", offsetof(MvProcessInfo, id));
    printf("%zu ", offsetof(MvProcessInfo, parent));
    printf("%zu ", offsetof(MvProcessInfo, type));
    printf("%zu ", offsetof(MvProcessInfo, priority));
    printf("%zu ", offsetof(MvProcessInfo, status));
    printf("%zu ", offsetof(MvProcessInfo, messages));
    printf("%zu ", offsetof(MvProcessInfo, creator_id));
    printf("%zu ", offsetof(MvProcessInfo, attached));
    printf("%zu %zu ", sizeof(MvProcessHandleInfo), _Alignof(MvProcessHandleInfo));
    printf("%zu ", offsetof(MvProcessHandleInfo, reference));
    printf("%zu ", offsetof(MvProcessHandleInfo, alive));
    printf("%zu ", offsetof(MvProcessHandleInfo, process));
    printf("%zu ", offsetof(MvProcessHandleInfo, reason));
    _Static_assert(sizeof(MvResult) == sizeof(int), "enum size");
    printf("%u ", (unsigned)MV_OK);
    printf("%u ", (unsigned)MV_INVALID_ARGUMENT);
    printf("%u ", (unsigned)MV_OVERFLOW);
    printf("%u ", (unsigned)MV_NOT_FOUND);
    printf("%u ", (unsigned)MV_OUT_OF_MEMORY);
    printf("%u ", (unsigned)MV_BUSY);
    printf("%u ", (unsigned)MV_PROCESS_ERROR);
    printf("%u ", (unsigned)MV_QUEUE_FULL);
    printf("%u ", (unsigned)MV_QUEUE_EMPTY);
    printf("%u ", (unsigned)MV_BUFFER_TOO_SMALL);
    _Static_assert(sizeof(MvSignal) == sizeof(int), "enum size");
    printf("%u ", (unsigned)MV_SIGNAL_SLEEP);
    printf("%u ", (unsigned)MV_SIGNAL_FREEZE);
    printf("%u ", (unsigned)MV_SIGNAL_WAKEUP);
    printf("%u ", (unsigned)MV_SIGNAL_KILL);
    _Static_assert(sizeof(MvStepResult) == sizeof(int), "enum size");
    printf("%u ", (unsigned)MV_STEP_YIELD);
    printf("%u ", (unsigned)MV_STEP_DONE);
    printf("%u ", (unsigned)MV_STEP_ERROR);
    _Static_assert(sizeof(MvProcessStatus) == sizeof(int), "enum size");
    printf("%u ", (unsigned)MV_PROCESS_RUNNING);
    printf("%u ", (unsigned)MV_PROCESS_SLEEPING);
    printf("%u ", (unsigned)MV_PROCESS_FROZEN);
    _Static_assert(sizeof(MvEndReason) == sizeof(int), "enum size");
    printf("%u ", (unsigned)MV_END_KILLED);
    printf("%u ", (unsigned)MV_END_RETURNED);
    printf("%u ", (unsigned)MV_END_ERROR);
    printf("%zu %zu %zu %zu %zu ", sizeof(MvLineageInfo), _Alignof(MvLineageInfo),
           offsetof(MvLineageInfo, id), offsetof(MvLineageInfo, creator_id), offsetof(MvLineageInfo, alive));
    printf("%zu %zu ", sizeof(MvRect), _Alignof(MvRect));
    printf("%zu ", offsetof(MvRect, x));
    printf("%zu ", offsetof(MvRect, y));
    printf("%zu ", offsetof(MvRect, width));
    printf("%zu ", offsetof(MvRect, height));
    printf("%zu %zu ", sizeof(MvCollisionShape), _Alignof(MvCollisionShape));
    printf("%zu ", offsetof(MvCollisionShape, kind));
    printf("%zu ", offsetof(MvCollisionShape, width));
    printf("%zu ", offsetof(MvCollisionShape, height));
    printf("%zu ", offsetof(MvCollisionShape, radius));
    printf("%zu ", offsetof(MvCollisionShape, offset_x));
    printf("%zu ", offsetof(MvCollisionShape, offset_y));
    printf("%zu %zu ", sizeof(MvCollisionTransform), _Alignof(MvCollisionTransform));
    printf("%zu ", offsetof(MvCollisionTransform, x));
    printf("%zu ", offsetof(MvCollisionTransform, y));
    printf("%zu ", offsetof(MvCollisionTransform, angle));
    printf("%zu ", offsetof(MvCollisionTransform, scale));
    printf("%zu %zu ", sizeof(MvCollisionSweep), _Alignof(MvCollisionSweep));
    printf("%zu ", offsetof(MvCollisionSweep, hit));
    printf("%zu ", offsetof(MvCollisionSweep, id));
    printf("%zu ", offsetof(MvCollisionSweep, fraction));
    printf("%zu ", offsetof(MvCollisionSweep, normal_x));
    printf("%zu ", offsetof(MvCollisionSweep, normal_y));
    _Static_assert(sizeof(MvCollisionKind) == sizeof(int), "collision enum size");
    printf("%u %u ", (unsigned)MV_COLLISION_CIRCLE, (unsigned)MV_COLLISION_RECT);
    printf("%zu %zu ", sizeof(MvCameraViewport), _Alignof(MvCameraViewport));
    printf("%zu ", offsetof(MvCameraViewport, x));
    printf("%zu ", offsetof(MvCameraViewport, y));
    printf("%zu ", offsetof(MvCameraViewport, width));
    printf("%zu ", offsetof(MvCameraViewport, height));
    printf("%zu %zu ", sizeof(MvCamera), _Alignof(MvCamera));
    printf("%zu ", offsetof(MvCamera, x));
    printf("%zu ", offsetof(MvCamera, y));
    printf("%zu ", offsetof(MvCamera, viewport));
    printf("%zu %zu ", sizeof(MvAnimationFrame), _Alignof(MvAnimationFrame));
    printf("%zu ", offsetof(MvAnimationFrame, graph_id));
    printf("%zu ", offsetof(MvAnimationFrame, duration_us));
    printf("%zu %zu ", sizeof(MvAnimationSample), _Alignof(MvAnimationSample));
    printf("%zu ", offsetof(MvAnimationSample, index));
    printf("%zu ", offsetof(MvAnimationSample, graph_id));
    printf("%zu %zu ", sizeof(MvAnimationState), _Alignof(MvAnimationState));
    printf("%zu ", offsetof(MvAnimationState, elapsed_us));
    printf("%zu ", offsetof(MvAnimationState, remainder));
    printf("%zu ", offsetof(MvAnimationState, tick_rate));
    printf("%zu %zu ", sizeof(MvProcessDrawInfo), _Alignof(MvProcessDrawInfo));
    printf("%zu ", offsetof(MvProcessDrawInfo, id));
    printf("%zu ", offsetof(MvProcessDrawInfo, z));
    return 0;
}
