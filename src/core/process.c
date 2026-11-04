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
#include "entropy.h"
#include "schedule.h"
#include <stdint.h>
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

typedef struct {
    MvProcessId id, parent, creator_id;
    Genealogy *lineage;
    uint64_t type;
    int64_t priority;
    MvProcessStatus status;
    MvProcessStep step;
    MvProcessDestroy destroy;
    void *user;
    int killed, attached;
    MvEndReason reason;
    MessageNode *head, *tail;
    size_t messages;
} Process;

typedef struct {
    MvProcessId id;
    MvEndReason reason;
} EndedProcess;

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
    size_t count, capacity;
    MvProcessId next_id;
    MvRuntimeId runtime_id;
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
    for (size_t i = 0; i < p->count; ++i) {
        MessageNode *node = p->items[i].head;
        while (node) {
            MessageNode *next = node->next;
            mv_message_release(&node->message);
            free(node);
            node = next;
        }
        if (p->items[i].destroy) p->items[i].destroy(p->items[i].user);
    }
    while (p->resources) {
        MvResource *resource = p->resources;
        resource->owner_reference = 0;
        if (!resource->references) free_resource(resource);
        else unlink_resource(resource);
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
        .status = MV_PROCESS_RUNNING, .step = step, .destroy = destroy, .user = user};
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
    size_t count = p->count;
    if (count > SIZE_MAX / sizeof(MalvadoScheduleItem)) return MV_OVERFLOW;
    MalvadoScheduleItem *order = count ? malloc(count * sizeof(*order)) : NULL;
    if (count && !order) return MV_OUT_OF_MEMORY;
    p->busy = 1;
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
    p->busy = 0;
    return result;
}
