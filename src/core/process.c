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
#include "process.h"
#include "malvado/collision.h"
#include "malvado/graphic.h"
#include "malvado/animation.h"
#include "malvado/camera.h"
#include "collision_internal.h"
#include "entropy.h"
#include "schedule.h"
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

typedef struct MessageNode {
    MvMessage message;
    struct MessageNode *next;
} MessageNode;

typedef struct Genealogy {
    MvProcessId id;
    struct Genealogy *creator, *owner, *previous, *next;
    size_t children;
    int alive, terminating;
} Genealogy;

typedef enum {
    COLLIDER_GRAPH,
    COLLIDER_FIXED,
    COLLIDER_DISABLED,
    COLLIDER_LOCAL
} ColliderMode;

typedef struct CameraNode {
    struct CameraNode *next;
    uint64_t id;
    MvCamera value;
} CameraNode;

typedef struct {
    MvProcessId id, parent, creator_id;
    Genealogy *lineage;
    uint64_t type;
    int64_t priority;
    double z;
    double x, y;
    double angle, scale;
    uint64_t camera_id;
    int has_region;
    MvCameraViewport region;
    float scroll_x, scroll_y;
    MvProcessStatus status;
    MvProcessStep step;
    MvProcessDestroy destroy;
    void *user;
    int killed, attached;
    int has_graphic;
    MvGraphic graphic;
    MvAnimation *animation;
    MvAnimationState animation_state;
    unsigned animation_rate;
    uint64_t animation_tick;
    void *graphic_data;
    MvResourceDestroy graphic_destroy;
    MvVisualSample visual_sample;
    ColliderMode collider_mode;
    int has_collider;
    MvCollisionShape collider;
    MvCollisionTransform collider_transform;
    MvCollisionBounds collider_bounds;
    MvEndReason reason;
    MessageNode *head, *tail;
    size_t messages;
} Process;

typedef struct {
    MvProcessId id;
    MvEndReason reason;
} EndedProcess;

struct MvProcessHandle {
    struct MvProcessHandle *next, *previous;
    struct MvProcesses *manager;
    MvProcessRef reference;
    MvEndReason reason;
};

struct MvResource {
    struct MvResource *next, *previous;
    struct MvProcesses *manager;
    MvProcessId owner;
    size_t references;
    int owner_reference;
    void *data;
    MvResourceDestroy destroy;
};

struct MvProcesses {
    MvContext *ctx;
    Process *items;
    EndedProcess *ended;
    size_t ended_count;
    Genealogy *lineage, *cleanup_lineage;
    MvResource *resources;
    MvCamera main_camera;
    CameraNode *cameras;
    uint64_t last_camera_id;
    MvProcessHandle *handles;
    size_t count, capacity;
    MvProcessId next_id;
    MvRuntimeId runtime_id;
    uint64_t ticks;
    int busy, closing;
};

static _Atomic uint64_t next_runtime_id = 1;

MvProcesses *mv_processes_create(MvContext *ctx)
{
    uint64_t random[2];
    if (!malvado_random_bytes(random, sizeof(random))) return NULL;
    uint64_t id = atomic_load_explicit(&next_runtime_id, memory_order_relaxed);
    do {
        if (id == UINT64_MAX) return NULL;
    } while (!atomic_compare_exchange_weak_explicit(&next_runtime_id, &id, id + 1,
                                                     memory_order_relaxed, memory_order_relaxed));
    MvProcesses *p = calloc(1, sizeof(*p));
    if (p) {
        p->ctx = ctx;
        p->last_camera_id = 1;
        p->runtime_id = (MvRuntimeId){random[0], random[1], id};
    }
    return p;
}

MvResult
mv_context_runtime_id(MvContext *ctx, MvRuntimeId *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out) return MV_INVALID_ARGUMENT;
    *out = p->runtime_id;
    return MV_OK;
}

MvResult
mv_process_next_id(MvContext *ctx, MvProcessId *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out) return MV_INVALID_ARGUMENT;
    if (p->next_id == UINT64_MAX) return MV_OVERFLOW;
    *out = p->next_id + 1;
    return MV_OK;
}

MvResult
mv_process_ref_create(MvContext *ctx, MvProcessId id, MvProcessRef *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out || !id) return MV_INVALID_ARGUMENT;
    if (id > p->next_id) return MV_NOT_FOUND;
    *out = (MvProcessRef){p->runtime_id, id};
    return MV_OK;
}

MvResult
mv_process_ref_resolve(MvContext *ctx, MvProcessRef ref, MvProcessId *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out || !ref.id || ref.runtime.high != p->runtime_id.high ||
        ref.runtime.low != p->runtime_id.low || ref.runtime.sequence != p->runtime_id.sequence)
        return MV_INVALID_ARGUMENT;
    if (ref.id > p->next_id) return MV_NOT_FOUND;
    *out = ref.id;
    return MV_OK;
}

int mv_processes_can_destroy(const MvProcesses *p)
{
    return p && !p->busy && !p->closing;
}

static Process *find(MvProcesses *p, MvProcessId id, int include_killed)
{
    if (!p || !id) return NULL;
    for (size_t i = 0; i < p->count; ++i)
        if (p->items[i].id == id && (include_killed || !p->items[i].killed))
            return &p->items[i];
    return NULL;
}

static MvResult
resolve_live_process(MvContext *ctx, MvProcessRef reference, Process **out)
{
    MvProcessId id;
    MvResult result = mv_process_ref_resolve(ctx, reference, &id);
    if (result != MV_OK)
        return result;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item)
        return MV_NOT_FOUND;
    *out = item;
    return MV_OK;
}

static MvResult
camera_value(MvProcesses *p, MvCameraRef reference, MvCamera **out)
{
    if (!p || !reference.id || reference.runtime.high != p->runtime_id.high ||
        reference.runtime.low != p->runtime_id.low || reference.runtime.sequence != p->runtime_id.sequence)
        return MV_INVALID_ARGUMENT;
    if (reference.id == 1) {
        *out = &p->main_camera;
        return MV_OK;
    }
    for (CameraNode *node = p->cameras; node; node = node->next) {
        if (node->id == reference.id) {
            *out = &node->value;
            return MV_OK;
        }
    }
    return MV_NOT_FOUND;
}

MvResult
mv_camera_main(MvContext *ctx, MvCameraRef *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out) return MV_INVALID_ARGUMENT;
    *out = (MvCameraRef){p->runtime_id, 1};
    return MV_OK;
}

static int
camera_copy_valid(MvCamera value)
{
    MvCamera checked;
    const MvCameraViewport *view = value.viewport.width || value.viewport.height ||
        value.viewport.x || value.viewport.y ? &value.viewport : NULL;
    return mv_camera_init(&checked, value.x, value.y, view);
}

MvResult
mv_camera_create(MvContext *ctx, MvCamera value, MvCameraRef *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out || !camera_copy_valid(value)) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    if (p->last_camera_id == UINT64_MAX - 1) return MV_OVERFLOW;
    CameraNode *node = malloc(sizeof(*node));
    if (!node) return MV_OUT_OF_MEMORY;
    *node = (CameraNode){p->cameras, ++p->last_camera_id, value};
    p->cameras = node;
    *out = (MvCameraRef){p->runtime_id, node->id};
    return MV_OK;
}

MvResult
mv_camera_get(MvContext *ctx, MvCameraRef reference, MvCamera *out)
{
    if (!out) return MV_INVALID_ARGUMENT;
    MvCamera *value;
    MvResult result = camera_value(mv_context_processes(ctx), reference, &value);
    if (result == MV_OK) *out = *value;
    return result;
}

MvResult
mv_camera_put(MvContext *ctx, MvCameraRef reference, MvCamera value)
{
    if (!camera_copy_valid(value)) return MV_INVALID_ARGUMENT;
    MvCamera *destination;
    MvResult result = camera_value(mv_context_processes(ctx), reference, &destination);
    if (result == MV_OK) *destination = value;
    return result;
}

MvResult
mv_camera_close(MvContext *ctx, MvCameraRef reference)
{
    MvProcesses *p = mv_context_processes(ctx);
    MvCamera *value;
    MvResult result = camera_value(p, reference, &value);
    if (result == MV_INVALID_ARGUMENT) return result;
    if (reference.id == 1) return MV_INVALID_ARGUMENT;
    if (reference.id > p->last_camera_id) return MV_NOT_FOUND;
    CameraNode **link = &p->cameras;
    while (*link && (*link)->id != reference.id) link = &(*link)->next;
    if (*link) {
        CameraNode *node = *link;
        *link = node->next;
        free(node);
    }
    return MV_OK;
}

MvResult
mv_process_camera_set(MvContext *ctx, MvProcessRef reference, const MvCameraRef *camera)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    uint64_t id = 0;
    if (camera) {
        if (!camera->id && !camera->runtime.high && !camera->runtime.low && !camera->runtime.sequence) {
            id = UINT64_MAX;
        } else {
            MvCamera *value;
            result = camera_value(mv_context_processes(ctx), *camera, &value);
            if (result != MV_OK) return result;
            id = camera->id;
        }
    }
    item->camera_id = id;
    return MV_OK;
}

MvResult
mv_process_camera_get(MvContext *ctx, MvProcessRef reference, MvCameraRef *out)
{
    if (!out) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (item->camera_id == UINT64_MAX) *out = (MvCameraRef){0};
    else *out = (MvCameraRef){mv_context_processes(ctx)->runtime_id, item->camera_id ? item->camera_id : 1};
    return MV_OK;
}

MvResult
mv_process_camera_resolve(MvContext *ctx, MvProcessRef reference, MvCamera *out)
{
    if (!out) return MV_INVALID_ARGUMENT;
    MvCameraRef camera;
    MvResult result = mv_process_camera_get(ctx, reference, &camera);
    if (result != MV_OK) return result;
    if (!camera.id) { *out = (MvCamera){0}; return MV_OK; }
    return mv_camera_get(ctx, camera, out);
}

MvResult
mv_process_region_set(MvContext *ctx, MvProcessRef reference, const MvCameraViewport *region)
{
    if (region && (region->width <= 0 || region->height <= 0)) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (region) item->region = *region;
    item->has_region = region != NULL;
    return MV_OK;
}

MvResult
mv_process_region_get(MvContext *ctx, MvProcessRef reference, MvCameraViewport *region, int *present)
{
    if (!region || !present || present == &region->x || present == &region->y ||
        present == &region->width || present == &region->height) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (item->has_region) *region = item->region;
    *present = item->has_region;
    return MV_OK;
}

MvResult
mv_process_scroll_set(MvContext *ctx, MvProcessRef reference, float x, float y)
{
    if (!isfinite(x) || !isfinite(y) || fabsf(x) > FLT_MAX / 16 || fabsf(y) > FLT_MAX / 16)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    item->scroll_x = x;
    item->scroll_y = y;
    return MV_OK;
}

MvResult
mv_process_scroll_get(MvContext *ctx, MvProcessRef reference, float *x, float *y)
{
    if (!x || !y || x == y) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    *x = item->scroll_x;
    *y = item->scroll_y;
    return MV_OK;
}

MvResult
mv_process_view_clip(MvContext *ctx, MvProcessRef reference, const MvCameraViewport *host,
                     int width, int height, MvCameraViewport *out)
{
    if (!out || width <= 0 || height <= 0 || (host && (host->width <= 0 || host->height <= 0)))
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    MvCamera camera;
    result = mv_process_camera_resolve(ctx, reference, &camera);
    if (result != MV_OK) return result;
    MvCameraViewport clipped;
    int visible = mv_camera_clip(&camera, item->has_region ? &item->region : NULL, width, height, &clipped);
    if (visible < 0) return MV_INVALID_ARGUMENT;
    if (visible && host) {
        MvCamera intersection = {.viewport = clipped};
        if (mv_camera_clip(&intersection, host, width, height, &clipped) < 0)
            return MV_INVALID_ARGUMENT;
    }
    *out = clipped;
    return MV_OK;
}

MvResult
mv_process_view_offset(MvContext *ctx, MvProcessRef reference, double *x, double *y)
{
    if (!x || !y || x == y) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    MvCamera camera;
    result = mv_process_camera_resolve(ctx, reference, &camera);
    if (result != MV_OK) return result;
    return mv_camera_project(&camera, 0, 0, item->scroll_x, item->scroll_y, x, y) ? MV_OK : MV_INVALID_ARGUMENT;
}

MvResult
mv_process_animation_clock_set(MvContext *ctx, MvProcessRef reference,
                         MvAnimation *animation, unsigned tick_rate)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (item->visual_sample) return MV_BUSY;
    if (!animation || !tick_rate || tick_rate > 1000000) return MV_INVALID_ARGUMENT;
    if (!mv_animation_retain(animation)) return MV_OVERFLOW;
    mv_animation_destroy(item->animation);
    item->animation = animation;
    item->animation_state = (MvAnimationState){0};
    item->animation_rate = tick_rate;
    item->animation_tick = mv_context_processes(ctx)->ticks;
    return MV_OK;
}

MvResult
mv_process_animation_clock_get(MvContext *ctx, MvProcessRef reference,
                         MvAnimationState *state, MvAnimationSample *sample, int *present)
{
    if (!state || !sample || !present) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (item->animation) {
        MvAnimationSample value;
        if (!mv_animation_sample(item->animation, item->animation_state.elapsed_us, &value))
            return MV_INVALID_ARGUMENT;
        *state = item->animation_state;
        *sample = value;
    }
    *present = item->animation != NULL;
    return MV_OK;
}

MvResult
mv_process_animation_clock_clear(MvContext *ctx, MvProcessRef reference)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK) return result;
    if (item->visual_sample) return MV_BUSY;
    mv_animation_destroy(item->animation);
    item->animation = NULL;
    item->animation_state = (MvAnimationState){0};
    return MV_OK;
}

static void
release_graphic_data(Process *item)
{
    if (item->visual_sample) {
        mv_animation_destroy(item->animation);
        item->animation = NULL;
        item->animation_state = (MvAnimationState){0};
        item->visual_sample = NULL;
    }
    void *data = item->graphic_data;
    MvResourceDestroy destroy = item->graphic_destroy;
    item->graphic_data = NULL;
    item->graphic_destroy = NULL;
    if (destroy) destroy(data);
}

MvResult
malvado_process_visual_set(MvContext *ctx, MvProcessRef ref,
                          MvCollisionShape shape, void *data, MvResourceDestroy destroy)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, ref, &item);
    if (result != MV_OK) return result;
    MvCollisionBounds bounds;
    if (!data || !destroy || !mv_collision_bounds(shape, (MvCollisionTransform){0, 0, 0, 1}, &bounds))
        return MV_INVALID_ARGUMENT;
    release_graphic_data(item);
    item->graphic = (MvGraphic){.shape = shape};
    item->has_graphic = 1;
    item->graphic_data = data;
    item->graphic_destroy = destroy;
    return MV_OK;
}

MvResult
malvado_process_visual_animation_set(MvContext *ctx, MvProcessRef ref,
    MvAnimation *animation, unsigned rate, void *data, MvResourceDestroy destroy,
    MvVisualSample sample)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, ref, &item);
    if (result != MV_OK) return result;
    MvCollisionShape shape;
    MvCollisionBounds bounds;
    if (!animation || !rate || rate > 1000000 || !data || !destroy || !sample ||
        !sample(data, 0, &shape) ||
        !mv_collision_bounds(shape, (MvCollisionTransform){0,0,0,1}, &bounds))
        return MV_INVALID_ARGUMENT;
    if (!mv_animation_retain(animation)) return MV_OVERFLOW;
    release_graphic_data(item);
    mv_animation_destroy(item->animation);
    item->animation = animation;
    item->animation_state = (MvAnimationState){0};
    item->animation_rate = rate;
    item->animation_tick = mv_context_processes(ctx)->ticks;
    item->graphic = (MvGraphic){.shape = shape};
    item->has_graphic = 1;
    item->graphic_data = data;
    item->graphic_destroy = destroy;
    item->visual_sample = sample;
    return MV_OK;
}

MvResult
malvado_process_visual_get(MvContext *ctx, MvProcessRef ref, void **data)
{
    if (!data) return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, ref, &item);
    if (result != MV_OK) return result;
    *data = item->graphic_data;
    return MV_OK;
}

MvResult
mv_process_graphic_set(MvContext *ctx, MvProcessRef reference, MvGraphic graphic)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    MvCollisionShape checked;
    MvCollisionShape shape = graphic.shape;
    if (shape.kind == MV_COLLISION_CIRCLE) {
        if (shape.offset_x != 0 || shape.offset_y != 0 ||
            !mv_graphic_circle(shape.radius, &checked))
            return MV_INVALID_ARGUMENT;
    } else if (shape.kind == MV_COLLISION_RECT) {
        int top_left = shape.offset_x != 0 || shape.offset_y != 0;
        if (!mv_graphic_rect(shape.width, shape.height, top_left, &checked) ||
            checked.offset_x != shape.offset_x || checked.offset_y != shape.offset_y)
            return MV_INVALID_ARGUMENT;
    } else {
        return MV_INVALID_ARGUMENT;
    }
    graphic.shape = checked;
    release_graphic_data(item);
    item->graphic = graphic;
    item->has_graphic = 1;
    return MV_OK;
}

MvResult
mv_process_graphic_get(MvContext *ctx, MvProcessRef reference, MvGraphic *graphic, int *present)
{
    if (!graphic || !present)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    if (item->has_graphic && !item->graphic_data)
        *graphic = item->graphic;
    *present = item->has_graphic && !item->graphic_data;
    return MV_OK;
}

MvResult
mv_process_graphic_clear(MvContext *ctx, MvProcessRef reference)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    release_graphic_data(item);
    item->has_graphic = 0;
    return MV_OK;
}

/* Fixed colliders keep their explicit transform. Bound shapes derive current
 * geometry/bounds at query time, independent of ticks or rendering. */
static MvResult
refresh_collider(Process *item)
{
    if (item->collider_mode == COLLIDER_FIXED || item->collider_mode == COLLIDER_DISABLED)
        return MV_OK;
    if (item->collider_mode == COLLIDER_GRAPH && !item->has_graphic) {
        item->has_collider = 0;
        return MV_OK;
    }
    MvCollisionShape shape = item->collider_mode == COLLIDER_GRAPH ?
        item->graphic.shape : item->collider;
    MvCollisionTransform transform = {item->x, item->y, item->angle, item->scale};
    MvCollisionBounds bounds;
    if (!mv_collision_bounds(shape, transform, &bounds)) {
        item->has_collider = 0;
        return MV_INVALID_ARGUMENT;
    }
    item->collider = shape;
    item->collider_transform = transform;
    item->collider_bounds = bounds;
    item->has_collider = 1;
    return MV_OK;
}

MvResult
mv_process_collider_bind(MvContext *ctx, MvProcessRef reference, const MvCollisionShape *override)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    int present = override || item->has_graphic;
    MvCollisionShape shape = {0};
    MvCollisionTransform transform = {item->x, item->y, item->angle, item->scale};
    MvCollisionBounds bounds;
    if (present) {
        shape = override ? *override : item->graphic.shape;
        if (!mv_collision_bounds(shape, transform, &bounds))
            return MV_INVALID_ARGUMENT;
    }
    item->collider_mode = override ? COLLIDER_LOCAL : COLLIDER_GRAPH;
    item->has_collider = present;
    if (present) {
        item->collider = shape;
        item->collider_transform = transform;
        item->collider_bounds = bounds;
    }
    return MV_OK;
}

MvResult
mv_process_collider_set(MvContext *ctx, MvProcessRef reference,
                        MvCollisionShape shape, MvCollisionTransform transform)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    MvCollisionBounds bounds;
    if (!mv_collision_bounds(shape, transform, &bounds))
        return MV_INVALID_ARGUMENT;
    item->collider_bounds = bounds;
    item->collider = shape;
    item->collider_transform = transform;
    item->has_collider = 1;
    item->collider_mode = COLLIDER_FIXED;
    return MV_OK;
}

MvResult
mv_process_collider_clear(MvContext *ctx, MvProcessRef reference)
{
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    item->has_collider = 0;
    item->collider_mode = COLLIDER_DISABLED;
    return MV_OK;
}

static int
collider_candidates(const Process *a, const Process *b)
{
    return fabs(a->collider_bounds.x - b->collider_bounds.x) <
               a->collider_bounds.half_width + b->collider_bounds.half_width &&
           fabs(a->collider_bounds.y - b->collider_bounds.y) <
               a->collider_bounds.half_height + b->collider_bounds.half_height;
}

MvResult
mv_process_collider_test(MvContext *ctx, MvProcessRef a, MvProcessRef b, int *overlap)
{
    if (!overlap)
        return MV_INVALID_ARGUMENT;
    Process *first, *second;
    MvResult result = resolve_live_process(ctx, a, &first);
    if (result != MV_OK)
        return result;
    result = resolve_live_process(ctx, b, &second);
    if (result != MV_OK)
        return result;
    result = refresh_collider(first);
    if (result != MV_OK)
        return result;
    result = refresh_collider(second);
    if (result != MV_OK)
        return result;
    *overlap = first != second && first->has_collider && second->has_collider &&
        collider_candidates(first, second) &&
        mv_collision_overlap(first->collider, first->collider_transform,
                             second->collider, second->collider_transform) == 1;
    return MV_OK;
}

MvResult
mv_process_collider_first(MvContext *ctx, MvProcessRef source, uint64_t type,
                          MvProcessRef *out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !out)
        return MV_INVALID_ARGUMENT;
    Process *origin;
    MvResult result = resolve_live_process(ctx, source, &origin);
    if (result != MV_OK)
        return result;
    result = refresh_collider(origin);
    if (result != MV_OK)
        return result;
    MvProcessRef found = {0};
    if (origin->has_collider) {
        for (size_t i = 0; i < p->count; ++i) {
            Process *item = &p->items[i];
            if (!item->id || item->killed || item == origin ||
                (type && item->type != type))
                continue;
            if (refresh_collider(item) != MV_OK || !item->has_collider)
                continue;
            if (!collider_candidates(origin, item))
                continue;
            if (mv_collision_overlap(origin->collider, origin->collider_transform,
                                     item->collider, item->collider_transform) == 1) {
                found = (MvProcessRef){p->runtime_id, item->id};
                break;
            }
        }
    }
    *out = found;
    return MV_OK;
}

MvResult
mv_process_collider_find(MvContext *ctx, MvProcessRef source, uint64_t type,
                         MvProcessRef *out, size_t capacity, size_t *total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !total || (capacity && !out))
        return MV_INVALID_ARGUMENT;
    Process *origin;
    MvResult result = resolve_live_process(ctx, source, &origin);
    if (result != MV_OK)
        return result;
    result = refresh_collider(origin);
    if (result != MV_OK)
        return result;
    size_t count = 0;
    if (origin->has_collider) {
        for (size_t i = 0; i < p->count; ++i) {
            Process *item = &p->items[i];
            if (!item->id || item->killed || item == origin ||
                (type && item->type != type))
                continue;
            if (refresh_collider(item) != MV_OK || !item->has_collider)
                continue;
            if (!collider_candidates(origin, item))
                continue;
            if (mv_collision_overlap(origin->collider, origin->collider_transform,
                                     item->collider, item->collider_transform) != 1)
                continue;
            if (count < capacity)
                out[count] = (MvProcessRef){p->runtime_id, item->id};
            ++count;
        }
    }
    *total = count;
    return MV_OK;
}

static void
unlink_handle(MvProcessHandle *handle)
{
    MvProcesses *p = handle->manager;
    if (!p) return;
    if (handle->previous) handle->previous->next = handle->next;
    else p->handles = handle->next;
    if (handle->next) handle->next->previous = handle->previous;
    handle->manager = NULL;
    handle->next = handle->previous = NULL;
}

MvResult
mv_process_handle_create(MvContext *ctx, MvProcessRef ref, MvProcessHandle **out)
{
    if (!out) return MV_INVALID_ARGUMENT;
    MvProcessId id;
    MvResult result = mv_process_ref_resolve(ctx, ref, &id);
    if (result != MV_OK) return result;
    MvProcesses *p = mv_context_processes(ctx);
    if (p->closing) return MV_BUSY;
    if (!find(p, id, 0)) return MV_NOT_FOUND;
    MvProcessHandle *handle = calloc(1, sizeof(*handle));
    if (!handle) return MV_OUT_OF_MEMORY;
    handle->manager = p;
    handle->reference = ref;
    handle->next = p->handles;
    if (p->handles) p->handles->previous = handle;
    p->handles = handle;
    *out = handle;
    return MV_OK;
}

MvResult
mv_process_handle_snapshot(const MvProcessHandle *handle, MvProcessHandleInfo *out)
{
    if (!handle || !out) return MV_INVALID_ARGUMENT;
    MvProcessHandleInfo result = {0};
    result.reference = handle->reference;
    if (!handle->manager) {
        result.reason = handle->reason;
    } else {
        Process *item = find(handle->manager, handle->reference.id, 1);
        if (!item) return MV_NOT_FOUND;
        if (item->killed) {
            result.reason = item->reason;
        } else {
            result.alive = 1;
            result.process = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                                             item->status, item->messages, item->creator_id, item->attached};
        }
    }
    *out = result;
    return MV_OK;
}

void
mv_process_handle_destroy(MvProcessHandle *handle)
{
    if (!handle) return;
    unlink_handle(handle);
    free(handle);
}

static void
retire_handles(MvProcesses *p, const Process *item)
{
    MvProcessHandle *handle = p->handles;
    while (handle) {
        MvProcessHandle *next = handle->next;
        if (handle->reference.id == item->id) {
            handle->reason = item->reason;
            unlink_handle(handle);
        }
        handle = next;
    }
}

int
mv_process_end_reason(MvContext *ctx, MvProcessId id, MvEndReason *reason)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !id || !reason) return 0;
    Process *item = find(p, id, 1);
    if (item && item->killed) {
        *reason = item->reason;
        return 1;
    }
    for (size_t i = 0; i < p->ended_count; ++i) {
        if (p->ended[i].id == id) {
            *reason = p->ended[i].reason;
            return 1;
        }
    }
    return 0;
}

static void
unlink_resource(MvResource *resource)
{
    MvProcesses *p = resource->manager;
    if (!p) return;
    if (resource->previous) resource->previous->next = resource->next;
    else p->resources = resource->next;
    if (resource->next) resource->next->previous = resource->previous;
    resource->manager = NULL;
    resource->next = resource->previous = NULL;
}

static void
free_resource(MvResource *resource)
{
    void *data = resource->data;
    MvResourceDestroy destroy = resource->destroy;
    unlink_resource(resource);
    free(resource);
    destroy(data);
}

MvResult
mv_resource_create(MvContext *ctx, const MvProcessRef *owner, void *data,
                   MvResourceDestroy destroy, MvResource **out)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !destroy || !out) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    MvProcessId owner_id = 0;
    if (owner) {
        MvResult result = mv_process_ref_resolve(ctx, *owner, &owner_id);
        if (result != MV_OK) return result;
        if (!find(p, owner_id, 0)) return MV_NOT_FOUND;
    }
    MvResource *resource = calloc(1, sizeof(*resource));
    if (!resource) return MV_OUT_OF_MEMORY;
    resource->manager = p;
    resource->owner = owner_id;
    resource->owner_reference = 1;
    resource->data = data;
    resource->destroy = destroy;
    resource->next = p->resources;
    if (p->resources) p->resources->previous = resource;
    p->resources = resource;
    *out = resource;
    return MV_OK;
}

MvResult
mv_resource_transfer(MvResource *resource, const MvProcessRef *owner)
{
    if (!resource || !resource->manager) return MV_INVALID_ARGUMENT;
    MvProcesses *p = resource->manager;
    if (p->closing) return MV_BUSY;
    MvProcessId id = 0;
    if (owner) {
        MvResult result = mv_process_ref_resolve(p->ctx, *owner, &id);
        if (result != MV_OK) return result;
        if (!find(p, id, 0)) return MV_NOT_FOUND;
    }
    resource->owner = id;
    resource->owner_reference = 1;
    return MV_OK;
}

MvResult
mv_resource_retain(MvResource *resource)
{
    if (!resource) return MV_INVALID_ARGUMENT;
    if (resource->references == SIZE_MAX) return MV_OVERFLOW;
    ++resource->references;
    return MV_OK;
}

MvResult
mv_resource_release(MvResource *resource)
{
    if (!resource || !resource->references) return MV_INVALID_ARGUMENT;
    --resource->references;
    if (!resource->references && !resource->owner_reference) free_resource(resource);
    return MV_OK;
}

void *
mv_resource_data(const MvResource *resource)
{
    return resource ? resource->data : NULL;
}

static void
release_owned_resources(MvProcesses *p, MvProcessId owner)
{
    for (;;) {
        MvResource *resource = p->resources;
        while (resource && (!resource->owner_reference || resource->owner != owner))
            resource = resource->next;
        if (!resource) break;
        resource->owner_reference = 0;
        if (!resource->references) free_resource(resource);
    }
}

static void
prune_lineage(MvProcesses *p, Genealogy *node)
{
    while (node && !node->alive && !node->children) {
        Genealogy *creator = node->creator;
        if (node->previous) node->previous->next = node->next;
        else p->lineage = node->next;
        if (node->next) node->next->previous = node->previous;
        if (creator) --creator->children;
        free(node);
        node = creator;
    }
}

static int
created_under(const Genealogy *node, MvProcessId ancestor)
{
    for (; node; node = node->creator)
        if (node->id == ancestor) return 1;
    return 0;
}

void mv_processes_destroy(MvProcesses *p)
{
    if (!p) return;
    p->closing = 1;
    for (size_t i = 0; i < p->count; ++i)
        p->items[i].killed = 1;
    for (size_t i = 0; i < p->count; ++i)
        retire_handles(p, &p->items[i]);
    for (size_t i = 0; i < p->count; ++i) {
        MessageNode *node = p->items[i].head;
        while (node) {
            MessageNode *next = node->next;
            mv_message_release(&node->message);
            free(node);
            node = next;
        }
        mv_animation_destroy(p->items[i].animation);
        p->items[i].animation = NULL;
        release_graphic_data(&p->items[i]);
        if (p->items[i].destroy) p->items[i].destroy(p->items[i].user);
    }
    while (p->resources) {
        MvResource *resource = p->resources;
        resource->owner_reference = 0;
        if (!resource->references) free_resource(resource);
        else unlink_resource(resource);
    }
    while (p->cameras) {
        CameraNode *next = p->cameras->next;
        free(p->cameras);
        p->cameras = next;
    }
    free(p->items);
    free(p->ended);
    while (p->lineage) {
        Genealogy *next = p->lineage->next;
        free(p->lineage);
        p->lineage = next;
    }
    free(p);
}

static MvResult spawn_related(MvContext *ctx, MvProcessId creator_id, MvProcessId parent,
                                  int64_t priority,
                          MvProcessStep step, MvProcessDestroy destroy, void *user,
                          MvProcessId *id, int attached)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !step || !id) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    if (p->cleanup_lineage && parent) return MV_BUSY;
    Process *creator = creator_id ? find(p, creator_id, 0) : NULL;
    Genealogy *closing_creator = p->cleanup_lineage;
    if ((creator_id && !creator &&
         (!closing_creator || closing_creator->id != creator_id || parent)) ||
        (parent && !find(p, parent, 0))) return MV_NOT_FOUND;
    if (p->next_id == UINT64_MAX) return MV_OVERFLOW;
    if (p->count == p->capacity) {
        if (p->capacity > SIZE_MAX / 2 / sizeof(*p->items) ||
            p->capacity > SIZE_MAX / 2 / sizeof(*p->ended)) return MV_OVERFLOW;
        size_t capacity = p->capacity ? p->capacity * 2 : 16;
        Process *items = realloc(p->items, capacity * sizeof(*items));
        if (!items) return MV_OUT_OF_MEMORY;
        p->items = items;
        EndedProcess *ended = realloc(p->ended, capacity * sizeof(*ended));
        if (!ended) return MV_OUT_OF_MEMORY;
        p->ended = ended;
        p->capacity = capacity;
    }
    Genealogy *lineage = calloc(1, sizeof(*lineage));
    if (!lineage) return MV_OUT_OF_MEMORY;
    /* Growing the process array can move its entries. */
    creator = creator_id ? find(p, creator_id, 0) : NULL;
    Process *owner = parent ? find(p, parent, 0) : NULL;
    lineage->owner = owner ? owner->lineage : NULL;
    lineage->id = ++p->next_id;
    lineage->alive = 1;
    lineage->creator = creator ? creator->lineage : (creator_id ? closing_creator : NULL);
    if (lineage->creator) ++lineage->creator->children;
    lineage->next = p->lineage;
    if (p->lineage) p->lineage->previous = lineage;
    p->lineage = lineage;
    MvProcessId next = lineage->id;
    p->items[p->count++] = (Process){.id = next, .parent = parent, .creator_id = creator_id,
        .lineage = lineage, .priority = priority, .attached = attached,
        .scale = 1, .status = MV_PROCESS_RUNNING, .step = step, .destroy = destroy, .user = user};
    *id = next;
    return MV_OK;
}

MvResult mv_process_spawn(MvContext *ctx, MvProcessId parent, int64_t priority,
                          MvProcessStep step, MvProcessDestroy destroy, void *user,
                          MvProcessId *id)
{
    return spawn_related(ctx, parent, parent, priority, step, destroy, user, id, parent != 0);
}

MvResult mv_process_spawn_related(MvContext *ctx, MvProcessId creator_id, MvProcessId owner_id,
                                  int64_t priority, MvProcessStep step,
                                  MvProcessDestroy destroy, void *user, MvProcessId *id)
{
    return spawn_related(ctx, creator_id, owner_id, priority, step, destroy, user, id,
                         owner_id != 0);
}

void mv_message_release(MvMessage *message)
{
    if (!message) return;
    if (message->type == MV_MESSAGE_BYTES) free(message->value.bytes.data);
    if (message->type == MV_MESSAGE_OPAQUE && message->value.opaque.destroy)
        message->value.opaque.destroy(message->value.opaque.data);
    memset(message, 0, sizeof(*message));
}

MvResult mv_process_send(MvContext *ctx, MvProcessId id, const MvMessage *message)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !message || message->type < MV_MESSAGE_BOOLEAN || message->type > MV_MESSAGE_OPAQUE)
        return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    Process *item = find(p, id, 0);
    if (!item) return MV_NOT_FOUND;
    if (item->messages == 1024) return MV_QUEUE_FULL;
    if (message->type == MV_MESSAGE_BYTES && message->value.bytes.size && !message->value.bytes.data)
        return MV_INVALID_ARGUMENT;
    MessageNode *node = malloc(sizeof(*node));
    if (!node) return MV_OUT_OF_MEMORY;
    node->message = *message;
    node->next = NULL;
    if (message->type == MV_MESSAGE_BYTES) {
        size_t size = message->value.bytes.size;
        node->message.value.bytes.data = size ? malloc(size) : NULL;
        if (size && !node->message.value.bytes.data) { free(node); return MV_OUT_OF_MEMORY; }
        if (size) memcpy(node->message.value.bytes.data, message->value.bytes.data, size);
    }
    if (item->tail) item->tail->next = node;
    else item->head = node;
    item->tail = node;
    item->messages++;
    return MV_OK;
}

MvResult mv_process_receive(MvContext *ctx, MvProcessId id, MvMessage *message)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !message) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    Process *item = find(p, id, 0);
    if (!item) return MV_NOT_FOUND;
    MessageNode *node = item->head;
    if (!node) return MV_QUEUE_EMPTY;
    item->head = node->next;
    if (!item->head) item->tail = NULL;
    item->messages--;
    *message = node->message;
    free(node);
    return MV_OK;
}

MvResult mv_process_message_count(MvContext *ctx, MvProcessId id, size_t *count)
{
    if (!ctx || !count) return MV_INVALID_ARGUMENT;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item) return MV_NOT_FOUND;
    *count = item->messages;
    return MV_OK;
}

static int descendant(Process *item, MvProcessId id)
{
    /* A live process has only live owners. Owner links of dead nodes are not
     * retained for history and must not be followed from a dead process. */
    for (Genealogy *owner = item->lineage->owner; owner; owner = owner->owner)
        if (owner->id == id) return 1;
    return 0;
}

/* Owners always precede their children in creation order. */
static void kill_owned(MvProcesses *p, MvProcessId id)
{
    Process *root = find(p, id, 1);
    if (!root) return;
    root->killed = 1;
    root->lineage->terminating = 1;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->killed && item->attached && item->lineage->owner &&
            item->lineage->owner->terminating) {
            item->killed = 1;
            item->lineage->terminating = 1;
        }
    }
}

static void
apply_signal(MvProcesses *p, Process *item, MvSignal signal)
{
    if (item->killed) return;
    switch (signal) {
    case MV_SIGNAL_SLEEP: item->status = MV_PROCESS_SLEEPING; break;
    case MV_SIGNAL_FREEZE: item->status = MV_PROCESS_FROZEN; break;
    case MV_SIGNAL_WAKEUP: item->status = MV_PROCESS_RUNNING; break;
    case MV_SIGNAL_KILL: kill_owned(p, item->id); break;
    }
}

/* No callback or cleanup runs while this snapshot is built or applied. */
static MvResult
signal_tree_snapshot(MvProcesses *p, MvProcessId root, MvSignal signal, int creation)
{
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->id || item->killed) continue;
        if (creation ? created_under(item->lineage, root) :
                       (item->id == root || descendant(item, root))) ++count;
    }
    if (count > SIZE_MAX / sizeof(size_t)) return MV_OVERFLOW;
    size_t *targets = count ? malloc(count * sizeof(*targets)) : NULL;
    if (count && !targets) return MV_OUT_OF_MEMORY;
    size_t next = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->id || item->killed) continue;
        if (creation ? created_under(item->lineage, root) :
                       (item->id == root || descendant(item, root))) targets[next++] = i;
    }
    for (size_t i = 0; i < count; ++i)
        apply_signal(p, &p->items[targets[i]], signal);
    free(targets);
    return MV_OK;
}

MvResult mv_process_signal(MvContext *ctx, MvProcessId id, MvSignal signal, int tree)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || signal < MV_SIGNAL_SLEEP || signal > MV_SIGNAL_KILL) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    Process *item = find(p, id, 0);
    if (!item) return MV_NOT_FOUND;
    if (tree) return signal_tree_snapshot(p, id, signal, 0);
    apply_signal(p, item, signal);
    return MV_OK;
}

MvResult mv_process_kill(MvContext *ctx, MvProcessRef ref, int *killed)
{
    MvProcesses *p = mv_context_processes(ctx);
    MvProcessId id;
    if (!p || !killed) return MV_INVALID_ARGUMENT;
    MvResult result = mv_process_ref_resolve(ctx, ref, &id);
    if (result != MV_OK) return result;
    if (p->closing) return MV_BUSY;
    if (!find(p, id, 0)) {
        *killed = 0;
        return MV_OK;
    }
    kill_owned(p, id);
    *killed = 1;
    return MV_OK;
}

MvResult mv_process_priority(MvContext *ctx, MvProcessId id, int64_t *priority)
{
    if (!ctx || !priority) return MV_INVALID_ARGUMENT;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item) return MV_NOT_FOUND;
    *priority = item->priority;
    return MV_OK;
}

MvResult mv_process_set_priority(MvContext *ctx, MvProcessId id, int64_t priority)
{
    if (!ctx) return MV_INVALID_ARGUMENT;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item) return MV_NOT_FOUND;
    item->priority = priority;
    return MV_OK;
}

MvResult mv_process_set_type(MvContext *ctx, MvProcessId id, uint64_t type)
{
    if (!ctx) return MV_INVALID_ARGUMENT;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item) return MV_NOT_FOUND;
    item->type = type;
    return MV_OK;
}

MvResult mv_process_status(MvContext *ctx, MvProcessId id, MvProcessStatus *status)
{
    if (!ctx || !status) return MV_INVALID_ARGUMENT;
    Process *item = find(mv_context_processes(ctx), id, 0);
    if (!item) return MV_NOT_FOUND;
    *status = item->status;
    return MV_OK;
}

MvResult
mv_process_z(MvContext *ctx, MvProcessRef reference, double *out)
{
    if (!out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    *out = item->z;
    return MV_OK;
}

MvResult
mv_process_set_z(MvContext *ctx, MvProcessRef reference, double z)
{
    if (!isfinite(z))
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    item->z = z;
    return MV_OK;
}

MvResult
mv_process_position(MvContext *ctx, MvProcessRef reference, double *x, double *y)
{
    if (!x || !y || x == y)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    *x = item->x;
    *y = item->y;
    return MV_OK;
}

MvResult
mv_process_set_position(MvContext *ctx, MvProcessRef reference, double x, double y)
{
    if (!isfinite(x) || !isfinite(y))
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    item->x = x;
    item->y = y;
    return MV_OK;
}

MvResult
mv_process_angle(MvContext *ctx, MvProcessRef reference, double *out)
{
    if (!out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    *out = item->angle;
    return MV_OK;
}

MvResult
mv_process_set_angle(MvContext *ctx, MvProcessRef reference, double value)
{
    if (!isfinite(value))
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    item->angle = value;
    return MV_OK;
}

MvResult
mv_process_scale(MvContext *ctx, MvProcessRef reference, double *out)
{
    if (!out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    *out = item->scale;
    return MV_OK;
}

MvResult
mv_process_set_scale(MvContext *ctx, MvProcessRef reference, double value)
{
    if (!isfinite(value))
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    item->scale = value;
    return MV_OK;
}

MvResult
mv_process_world_query(MvContext *ctx, MvProcessRef reference,
                       const MvCollisionWorld *world, const MvCollisionShape *shape,
                       size_t *out)
{
    if (!world || !out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    size_t id = 0;
    if (shape) {
        MvCollisionTransform transform = {item->x, item->y, item->angle, item->scale};
        if (mv_collision_world_query(world, *shape, transform, &id) < 0)
            return MV_INVALID_ARGUMENT;
    }
    *out = id;
    return MV_OK;
}

MvResult
mv_process_world_move(MvContext *ctx, MvProcessRef reference,
                      const MvCollisionWorld *world, const MvCollisionShape *shape,
                      double dx, double dy, MvCollisionSweep *out)
{
    if (!world || !out || !isfinite(dx) || !isfinite(dy) ||
        fabs(dx) > DBL_MAX / 16 || fabs(dy) > DBL_MAX / 16)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    MvCollisionTransform transform = {item->x, item->y, item->angle, item->scale};
    MvCollisionSweep sweep = {.fraction = 1};
    if (shape) {
        if (mv_collision_world_move(world, *shape, &transform, dx, dy, &sweep) < 0)
            return MV_INVALID_ARGUMENT;
    } else {
        transform.x += dx;
        transform.y += dy;
    }
    if (!isfinite(transform.x) || !isfinite(transform.y))
        return MV_INVALID_ARGUMENT;
    item->x = transform.x;
    item->y = transform.y;
    *out = sweep;
    return MV_OK;
}

MvResult
mv_process_world_query_selected(MvContext *ctx, MvProcessRef reference,
                                const MvCollisionWorld *world, size_t *out)
{
    if (!world || !out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    result = refresh_collider(item);
    if (result != MV_OK)
        return result;
    size_t id = 0;
    if (item->has_collider &&
        mv_collision_world_query(world, item->collider, item->collider_transform, &id) < 0)
        return MV_INVALID_ARGUMENT;
    *out = id;
    return MV_OK;
}

MvResult
mv_process_world_move_selected(MvContext *ctx, MvProcessRef reference,
                               const MvCollisionWorld *world, double dx, double dy,
                               MvCollisionSweep *out)
{
    if (!world || !out || !isfinite(dx) || !isfinite(dy) ||
        fabs(dx) > DBL_MAX / 16 || fabs(dy) > DBL_MAX / 16)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    result = refresh_collider(item);
    if (result != MV_OK)
        return result;
    MvCollisionSweep sweep = {.fraction = 1};
    double x = item->x + dx, y = item->y + dy;
    MvCollisionTransform transform = item->collider_transform;
    MvCollisionBounds bounds;
    if (item->has_collider) {
        if (mv_collision_world_move(world, item->collider, &transform, dx, dy, &sweep) < 0)
            return MV_INVALID_ARGUMENT;
        x = transform.x;
        y = transform.y;
        if (item->collider_mode == COLLIDER_FIXED) {
            /* Preserve the fixed collider's offset from the process origin.
             * Validate both destinations before committing either one. */
            if (item->x != item->collider_transform.x)
                x = item->x + (transform.x - item->collider_transform.x);
            if (item->y != item->collider_transform.y)
                y = item->y + (transform.y - item->collider_transform.y);
        }
        if (!mv_collision_bounds(item->collider, transform, &bounds))
            return MV_INVALID_ARGUMENT;
    }
    if (!isfinite(x) || !isfinite(y))
        return MV_INVALID_ARGUMENT;
    item->x = x;
    item->y = y;
    if (item->has_collider) {
        item->collider_transform = transform;
        item->collider_bounds = bounds;
    }
    *out = sweep;
    return MV_OK;
}

static int
draw_info_less(MvProcessDrawInfo left, MvProcessDrawInfo right)
{
    return left.z < right.z || (left.z == right.z && left.id < right.id);
}

/* In-place heap sort keeps the buffer query allocation-free on every libc.
 * The unique creation ID makes equal-depth ordering deterministic. */
static void
draw_sift_down(MvProcessDrawInfo *items, size_t count, size_t root)
{
    while (root < count / 2) {
        size_t child = root * 2 + 1;
        if (child + 1 < count && draw_info_less(items[child], items[child + 1]))
            ++child;
        if (!draw_info_less(items[root], items[child]))
            return;
        MvProcessDrawInfo swap = items[root];
        items[root] = items[child];
        items[child] = swap;
        root = child;
    }
}

static void
sort_draw_info(MvProcessDrawInfo *items, size_t count)
{
    for (size_t root = count / 2; root > 0; --root)
        draw_sift_down(items, count, root - 1);
    for (size_t end = count; end > 1; --end) {
        MvProcessDrawInfo swap = items[0];
        items[0] = items[end - 1];
        items[end - 1] = swap;
        draw_sift_down(items, end - 1, 0);
    }
}

MvResult
mv_process_draw_order(MvContext *ctx, MvProcessDrawInfo *out, size_t capacity, size_t *total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !total || (capacity && !out))
        return MV_INVALID_ARGUMENT;
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i)
        if (!p->items[i].killed && p->items[i].status != MV_PROCESS_SLEEPING)
            ++count;
    *total = count;
    if (!capacity)
        return MV_OK;
    if (capacity < count)
        return MV_BUFFER_TOO_SMALL;
    size_t index = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->killed && item->status != MV_PROCESS_SLEEPING)
            out[index++] = (MvProcessDrawInfo){item->id, item->z};
    }
    if (count > 1)
        sort_draw_info(out, count);
    return MV_OK;
}

MvResult
mv_process_info(MvContext *ctx, MvProcessRef reference, MvProcessInfo *out)
{
    if (!out)
        return MV_INVALID_ARGUMENT;
    Process *item;
    MvResult result = resolve_live_process(ctx, reference, &item);
    if (result != MV_OK)
        return result;
    *out = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                           item->status, item->messages, item->creator_id, item->attached};
    return MV_OK;
}

size_t mv_process_count(const MvContext *ctx)
{
    MvProcesses *p = mv_context_processes((MvContext *)ctx);
    size_t count = 0;
    if (p) for (size_t i = 0; i < p->count; ++i) count += !p->items[i].killed;
    return count;
}

MvResult mv_process_snapshot(MvContext *ctx, MvProcessInfo *out, size_t capacity, size_t *total)
{
    return mv_process_find(ctx, 0, out, capacity, total);
}

MvResult mv_process_find(MvContext *ctx, uint64_t type, MvProcessInfo *out,
                         size_t capacity, size_t *total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !total || (capacity && !out)) return MV_INVALID_ARGUMENT;
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (item->killed || !item->id || (type && item->type != type)) continue;
        if (count < capacity)
            out[count] = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                                         item->status, item->messages, item->creator_id, item->attached};
        ++count;
    }
    *total = count;
    return MV_OK;
}

static MvResult
snapshot_owned(MvContext *ctx, MvProcessId owner, uint64_t type, MvProcessInfo *out,
               size_t capacity, size_t *total, int recursive)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !owner || !total || (capacity && !out)) return MV_INVALID_ARGUMENT;
    if (!find(p, owner, 0)) return MV_NOT_FOUND;
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->id || item->killed || (type && item->type != type)) continue;
        if (recursive ? !descendant(item, owner) : item->parent != owner) continue;
        if (count < capacity)
            out[count] = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                                         item->status, item->messages, item->creator_id, item->attached};
        ++count;
    }
    *total = count;
    return MV_OK;
}

MvResult
mv_process_children(MvContext *ctx, MvProcessId owner, uint64_t type,
                    MvProcessInfo *out, size_t capacity, size_t *total)
{
    return snapshot_owned(ctx, owner, type, out, capacity, total, 0);
}

MvResult
mv_process_descendants(MvContext *ctx, MvProcessId owner, uint64_t type,
                       MvProcessInfo *out, size_t capacity, size_t *total)
{
    return snapshot_owned(ctx, owner, type, out, capacity, total, 1);
}

MvResult
mv_process_lineage_snapshot(MvContext *ctx, MvLineageInfo *out, size_t capacity, size_t *total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !total || (capacity && !out)) return MV_INVALID_ARGUMENT;
    Genealogy *node = p->lineage;
    while (node && node->next) node = node->next;
    size_t count = 0;
    for (; node; node = node->previous) {
        if (count < capacity) {
            out[count] = (MvLineageInfo){node->id, node->creator ? node->creator->id : 0,
                                         find(p, node->id, 0) != NULL};
        }
        ++count;
    }
    *total = count;
    return MV_OK;
}

MvResult
mv_process_inspection_snapshot(MvContext *ctx, MvProcessInfo *processes,
                               size_t process_capacity, size_t *process_total,
                               MvLineageInfo *lineage, size_t lineage_capacity,
                               size_t *lineage_total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !process_total || !lineage_total || process_total == lineage_total ||
        (process_capacity && !processes) || (lineage_capacity && !lineage))
        return MV_INVALID_ARGUMENT;
    size_t processes_needed = mv_process_count(ctx);
    size_t lineage_needed = 0;
    for (Genealogy *node = p->lineage; node; node = node->next) ++lineage_needed;
    *process_total = processes_needed;
    *lineage_total = lineage_needed;
    if (process_capacity < processes_needed || lineage_capacity < lineage_needed)
        return MV_BUFFER_TOO_SMALL;
    size_t copied_processes, copied_lineage;
    (void)mv_process_snapshot(ctx, processes, process_capacity, &copied_processes);
    (void)mv_process_lineage_snapshot(ctx, lineage, lineage_capacity, &copied_lineage);
    return MV_OK;
}

MvResult
mv_process_inspection_capture(MvContext *ctx, MvInspectionHeader *header,
                              MvProcessInfo *processes, size_t process_capacity,
                              size_t *process_total, MvLineageInfo *lineage,
                              size_t lineage_capacity, size_t *lineage_total)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !header) return MV_INVALID_ARGUMENT;
    MvResult result = mv_process_inspection_snapshot(ctx, processes, process_capacity,
                                                     process_total, lineage, lineage_capacity,
                                                     lineage_total);
    if (result == MV_OK)
        *header = (MvInspectionHeader){p->runtime_id, p->ticks};
    return result;
}

static MvResult
snapshot_creation(MvContext *ctx, MvProcessId ancestor, uint64_t type,
                  MvProcessInfo *out, size_t capacity, size_t *total, int direct)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !ancestor || !total || (capacity && !out)) return MV_INVALID_ARGUMENT;
    if (ancestor > p->next_id) return MV_NOT_FOUND;
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (!item->id || item->killed || item->id == ancestor ||
            (type && item->type != type) ||
            (direct ? item->creator_id != ancestor : !created_under(item->lineage, ancestor))) continue;
        if (count < capacity)
            out[count] = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                                         item->status, item->messages, item->creator_id, item->attached};
        ++count;
    }
    *total = count;
    return MV_OK;
}

MvResult
mv_process_creation_descendants(MvContext *ctx, MvProcessId ancestor, uint64_t type,
                                MvProcessInfo *out, size_t capacity, size_t *total)
{
    return snapshot_creation(ctx, ancestor, type, out, capacity, total, 0);
}

MvResult
mv_process_creation_children(MvContext *ctx, MvProcessId ancestor, uint64_t type,
                             MvProcessInfo *out, size_t capacity, size_t *total)
{
    return snapshot_creation(ctx, ancestor, type, out, capacity, total, 1);
}

MvResult
mv_process_signal_creation_tree(MvContext *ctx, MvProcessId ancestor, MvSignal signal)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !ancestor || signal < MV_SIGNAL_SLEEP || signal > MV_SIGNAL_KILL)
        return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    if (ancestor > p->next_id) return MV_NOT_FOUND;
    return signal_tree_snapshot(p, ancestor, signal, 1);
}

MvResult mv_process_tick(MvContext *ctx)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p) return MV_INVALID_ARGUMENT;
    if (p->busy || p->closing) return MV_BUSY;
    if (p->ticks == UINT64_MAX) return MV_OVERFLOW;
    size_t count = p->count;
    if (count > SIZE_MAX / sizeof(MalvadoScheduleItem)) return MV_OVERFLOW;
    MalvadoScheduleItem *order = count ? malloc(count * sizeof(*order)) : NULL;
    if (count && !order) return MV_OUT_OF_MEMORY;
    p->busy = 1;
    ++p->ticks;
    p->ended_count = 0;
    for (size_t i = 0; i < count; ++i) order[i] = (MalvadoScheduleItem){i, p->items[i].priority};
    malvado_schedule_sort(order, count);
    MvResult result = MV_OK;
    for (size_t i = 0; i < count; ++i) {
        size_t slot = order[i].slot;
        Process *item = &p->items[slot];
        if (item->killed || item->status != MV_PROCESS_RUNNING) continue;
        MvProcessStep step = item->step;
        MvProcessId id = item->id;
        void *user = item->user;
        MvStepResult outcome = step(ctx, id, user);
        /* A callback may spawn and reallocate the array. */
        if (outcome != MV_STEP_YIELD) {
            if (!p->items[slot].killed)
                p->items[slot].reason = outcome == MV_STEP_DONE ? MV_END_RETURNED : MV_END_ERROR;
            kill_owned(p, id);
        }
        if (outcome != MV_STEP_YIELD && outcome != MV_STEP_DONE) {
            result = MV_PROCESS_ERROR;
            break;
        }
    }
    free(order);
    /* Destructors can spawn; remove each killed process before calling one. */
    for (;;) {
        size_t i = 0;
        while (i < p->count && (!p->items[i].killed || !p->items[i].id)) ++i;
        if (i == p->count) break;
        MvProcessDestroy destroy = p->items[i].destroy;
        void *user = p->items[i].user;
        MessageNode *node = p->items[i].head;
        Genealogy *lineage = p->items[i].lineage;
        p->ended[p->ended_count++] = (EndedProcess){p->items[i].id, p->items[i].reason};
        retire_handles(p, &p->items[i]);
        p->items[i].id = 0;
        p->items[i].lineage = NULL;
        lineage->alive = 0;
        p->items[i].destroy = NULL;
        p->items[i].head = p->items[i].tail = NULL;
        while (node) {
            MessageNode *next = node->next;
            mv_message_release(&node->message);
            free(node);
            node = next;
        }
        mv_animation_destroy(p->items[i].animation);
        p->items[i].animation = NULL;
        release_graphic_data(&p->items[i]);
        p->cleanup_lineage = lineage;
        if (destroy) destroy(user);
        release_owned_resources(p, lineage->id);
        p->cleanup_lineage = NULL;
        prune_lineage(p, lineage);
    }
    for (size_t i = 0; i < p->count; ++i)
        if (p->items[i].parent && !find(p, p->items[i].parent, 0)) p->items[i].parent = 0;
    size_t write = 0;
    for (size_t i = 0; i < p->count; ++i)
        if (p->items[i].id) p->items[write++] = p->items[i];
    p->count = write;
    /* Cleanup callbacks may change another process's status or clock. Apply
     * the final end-of-tick state, including newly assigned-clock skips. */
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (item->animation && !item->killed && item->status == MV_PROCESS_RUNNING &&
            item->animation_tick < p->ticks) {
            if (!mv_animation_advance(item->animation, &item->animation_state, item->animation_rate))
                result = MV_INVALID_ARGUMENT;
            if (item->visual_sample) {
                MvAnimationSample sample;
                MvCollisionShape shape;
                if (!mv_animation_sample(item->animation, item->animation_state.elapsed_us, &sample) ||
                    !item->visual_sample(item->graphic_data, sample.index, &shape))
                    result = MV_INVALID_ARGUMENT;
                else item->graphic.shape = shape;
            }
        }
    }
    p->busy = 0;
    return result;
}
