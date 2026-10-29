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
#include "schedule.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct MessageNode {
    MvMessage message;
    struct MessageNode *next;
} MessageNode;

typedef struct {
    MvProcessId id, parent;
    uint64_t type;
    int64_t priority;
    MvProcessStatus status;
    MvProcessStep step;
    MvProcessDestroy destroy;
    void *user;
    int killed;
    MessageNode *head, *tail;
    size_t messages;
} Process;

struct MvProcesses {
    MvContext *ctx;
    Process *items;
    size_t count, capacity;
    MvProcessId next_id;
    int busy, closing;
};

MvProcesses *mv_processes_create(MvContext *ctx)
{
    MvProcesses *p = calloc(1, sizeof(*p));
    if (p) p->ctx = ctx;
    return p;
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

void mv_processes_destroy(MvProcesses *p)
{
    if (!p) return;
    p->closing = 1;
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
    free(p->items);
    free(p);
}

MvResult mv_process_spawn(MvContext *ctx, MvProcessId parent, int64_t priority,
                          MvProcessStep step, MvProcessDestroy destroy, void *user,
                          MvProcessId *id)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !step || !id) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    if (parent && !find(p, parent, 0)) return MV_NOT_FOUND;
    if (p->next_id == UINT64_MAX) return MV_OVERFLOW;
    if (p->count == p->capacity) {
        if (p->capacity > SIZE_MAX / 2 / sizeof(*p->items)) return MV_OVERFLOW;
        size_t capacity = p->capacity ? p->capacity * 2 : 16;
        Process *items = realloc(p->items, capacity * sizeof(*items));
        if (!items) return MV_OUT_OF_MEMORY;
        p->items = items;
        p->capacity = capacity;
    }
    MvProcessId next = ++p->next_id;
    p->items[p->count++] = (Process){.id = next, .parent = parent, .priority = priority,
        .status = MV_PROCESS_RUNNING, .step = step, .destroy = destroy, .user = user};
    *id = next;
    return MV_OK;
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

static int descendant(MvProcesses *p, Process *item, MvProcessId id)
{
    MvProcessId parent = item->parent;
    while (parent) {
        if (parent == id) return 1;
        Process *ancestor = find(p, parent, 1);
        if (!ancestor) break;
        parent = ancestor->parent;
    }
    return 0;
}

MvResult mv_process_signal(MvContext *ctx, MvProcessId id, MvSignal signal, int tree)
{
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || signal < MV_SIGNAL_SLEEP || signal > MV_SIGNAL_KILL) return MV_INVALID_ARGUMENT;
    if (p->closing) return MV_BUSY;
    if (!find(p, id, 0)) return MV_NOT_FOUND;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (item->killed || (item->id != id && (!tree || !descendant(p, item, id)))) continue;
        switch (signal) {
        case MV_SIGNAL_SLEEP: item->status = MV_PROCESS_SLEEPING; break;
        case MV_SIGNAL_FREEZE: item->status = MV_PROCESS_FROZEN; break;
        case MV_SIGNAL_WAKEUP: item->status = MV_PROCESS_RUNNING; break;
        case MV_SIGNAL_KILL: item->killed = 1; break;
        }
    }
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
    MvProcesses *p = mv_context_processes(ctx);
    if (!p || !total || (capacity && !out)) return MV_INVALID_ARGUMENT;
    size_t count = 0;
    for (size_t i = 0; i < p->count; ++i) {
        Process *item = &p->items[i];
        if (item->killed || !item->id) continue;
        if (count < capacity)
            out[count] = (MvProcessInfo){item->id, item->parent, item->type, item->priority,
                                         item->status, item->messages};
        ++count;
    }
    *total = count;
    return MV_OK;
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
        if (outcome != MV_STEP_YIELD) p->items[slot].killed = 1;
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
        p->items[i].id = 0;
        p->items[i].destroy = NULL;
        p->items[i].head = p->items[i].tail = NULL;
        while (node) {
            MessageNode *next = node->next;
            mv_message_release(&node->message);
            free(node);
            node = next;
        }
        if (destroy) destroy(user);
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
