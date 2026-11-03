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

#ifndef MALVADO_H
#define MALVADO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvContext MvContext;

/* Cadence for a 60-tick-per-second scheduler. Initialize to {0}. Run the
 * first activation yourself, then call once per eligible tick. A rate of 0
 * or 60 runs every tick. Return 1 when due, 0 when not due, or -1 for an
 * invalid rate, state, or pointer. Invalid calls do not change the state. */
typedef struct { unsigned phase; } MvCadence;
int mv_cadence_step(MvCadence *cadence, unsigned fps);

typedef enum {
    MV_OK = 0,
    MV_INVALID_ARGUMENT,
    MV_OVERFLOW,
    MV_NOT_FOUND,
    MV_OUT_OF_MEMORY,
    MV_BUSY,
    MV_PROCESS_ERROR,
    MV_QUEUE_FULL,
    MV_QUEUE_EMPTY,
    MV_BUFFER_TOO_SMALL
} MvResult;

typedef uint64_t MvProcessId;
/* Copyable identities. Treat their fields as opaque. */
typedef struct { uint64_t high, low, sequence; } MvRuntimeId;
typedef struct { MvRuntimeId runtime; MvProcessId id; } MvProcessRef;
typedef struct MvResource MvResource;
typedef void (*MvResourceDestroy)(void *data);
typedef enum { MV_PROCESS_RUNNING, MV_PROCESS_SLEEPING, MV_PROCESS_FROZEN } MvProcessStatus;
typedef enum { MV_SIGNAL_SLEEP, MV_SIGNAL_FREEZE, MV_SIGNAL_WAKEUP, MV_SIGNAL_KILL } MvSignal;
typedef enum { MV_STEP_YIELD, MV_STEP_DONE, MV_STEP_ERROR } MvStepResult;
typedef MvStepResult (*MvProcessStep)(MvContext *ctx, MvProcessId id, void *user);
typedef void (*MvProcessDestroy)(void *user);
typedef struct {
    MvProcessId id, parent;
    uint64_t type;
    int64_t priority;
    MvProcessStatus status;
    size_t messages;
    MvProcessId creator_id;
    int attached;
} MvProcessInfo;

typedef struct {
    MvProcessId id, creator_id;
    int alive;
} MvLineageInfo;

typedef enum {
    MV_MESSAGE_BOOLEAN, MV_MESSAGE_INTEGER, MV_MESSAGE_NUMBER,
    MV_MESSAGE_BYTES, MV_MESSAGE_OPAQUE
} MvMessageType;
typedef struct {
    MvMessageType type;
    union {
        int boolean;
        int64_t integer;
        double number;
        struct { void *data; size_t size; } bytes;
        struct { void *data; void (*destroy)(void *); } opaque;
    } value;
} MvMessage;

/** Create a resource owned by a live process, or by the context if owner is
 * NULL. A process owner must belong to ctx. A successful call transfers data to destroy and returns a borrowed
 * resource pointer. On failure data remains with the caller and out is not
 * changed. The owner holds one reference until cleanup or context shutdown.
 * Use retain to acquire a separate reference before sharing. Each successful
 * retain needs one release. References may outlive the context; release them
 * before shutting down any platform service needed by destroy. All calls and
 * destructors use the context owner thread. */
MvResult mv_resource_create(MvContext *ctx, const MvProcessRef *owner, void *data,
                            MvResourceDestroy destroy, MvResource **out);
/** Move the owner reference to a live process in the same context, or to the
 * context when owner is NULL. Existing retained references do not change.
 * A resource retained after its owner ends can be assigned a new owner while
 * its context is live. Failure leaves its ownership unchanged. */
MvResult mv_resource_transfer(MvResource *resource, const MvProcessRef *owner);
MvResult mv_resource_retain(MvResource *resource);
MvResult mv_resource_release(MvResource *resource);
void *mv_resource_data(const MvResource *resource);

/** FIFO queue with at most 1024 messages per process. Bytes are copied on send;
 * opaque values transfer ownership only on successful send. Receive transfers
 * ownership of bytes or opaque values to the caller, who must call release. */
MvResult mv_process_send(MvContext *ctx, MvProcessId id, const MvMessage *message);
MvResult mv_process_receive(MvContext *ctx, MvProcessId id, MvMessage *message);
MvResult mv_process_message_count(MvContext *ctx, MvProcessId id, size_t *count);
void mv_message_release(MvMessage *message);

/** Copy the runtime identity. On failure the output is unchanged. */
MvResult mv_context_runtime_id(MvContext *ctx, MvRuntimeId *out);

/** Create or validate a context-scoped process reference. References remain
 * valid after the process ends, but cannot be used with a different context.
 * On failure the output is unchanged. The context must be live. */
MvResult mv_process_ref_create(MvContext *ctx, MvProcessId id, MvProcessRef *out);
MvResult mv_process_ref_resolve(MvContext *ctx, MvProcessRef ref, MvProcessId *out);

/** The process retains user until removal. If destroy is provided, a successful
 * spawn transfers responsibility for user to that destructor, which is called
 * once on removal or context destruction. Otherwise the caller must keep user
 * valid for the process lifetime. On failure the caller retains responsibility
 * for user and id is unchanged. parent=0 creates a root process. A nonzero
 * parent owns the new process, which ends when its owner ends. */
MvResult mv_process_spawn(MvContext *ctx, MvProcessId parent, int64_t priority,
                          MvProcessStep step, MvProcessDestroy destroy, void *user,
                          MvProcessId *id);
/** Create a process with separate creator and owner IDs. Both IDs must name
 * live processes in ctx, or be zero. During a destructor, its own creator ID
 * is also valid for a new independent process. The owner controls the current parent
 * relation; creator_id is historical. A nonzero owner links the child to its
 * lifetime. Creation ancestry remains available while live descendants need it. */
MvResult mv_process_spawn_related(MvContext *ctx, MvProcessId creator_id, MvProcessId owner_id,
                                  int64_t priority, MvProcessStep step,
                                  MvProcessDestroy destroy, void *user, MvProcessId *id);
MvResult mv_process_signal(MvContext *ctx, MvProcessId id, MvSignal signal, int tree);
MvResult mv_process_priority(MvContext *ctx, MvProcessId id, int64_t *priority);
MvResult mv_process_set_priority(MvContext *ctx, MvProcessId id, int64_t priority);
/** Type IDs are caller-defined; zero means no type. */
MvResult mv_process_set_type(MvContext *ctx, MvProcessId id, uint64_t type);
MvResult mv_process_status(MvContext *ctx, MvProcessId id, MvProcessStatus *status);
size_t mv_process_count(const MvContext *ctx);
/** Copy up to capacity live processes in creation order. total receives the
 * complete count even when the buffer is smaller; out may be NULL if capacity
 * is zero. Results are snapshots and do not expose internal storage. */
MvResult mv_process_snapshot(MvContext *ctx, MvProcessInfo *out, size_t capacity, size_t *total);
/** Copy live children or descendants of a live owner in creation order.
 * A type of zero selects all types. Sleeping and frozen processes are included.
 * The buffer and total follow mv_process_snapshot. Invalid input or an unknown
 * owner leaves both unchanged. These experimental functions currently use the
 * ownership relation only; use mv_process_creation_descendants for creation. */
MvResult mv_process_children(MvContext *ctx, MvProcessId owner, uint64_t type,
                             MvProcessInfo *out, size_t capacity, size_t *total);
MvResult mv_process_descendants(MvContext *ctx, MvProcessId owner, uint64_t type,
                                MvProcessInfo *out, size_t capacity, size_t *total);
/** Copy retained creation nodes in creation order. Dead nodes remain only while
 * needed by live descendants. Buffer rules match mv_process_snapshot. */
MvResult mv_process_lineage_snapshot(MvContext *ctx, MvLineageInfo *out,
                                     size_t capacity, size_t *total);
/** Capture live processes and retained creation nodes together. On insufficient
 * capacity, return MV_BUFFER_TOO_SMALL, write both required counts, and leave
 * both buffers unchanged. On invalid input, leave counts and buffers unchanged.
 * The caller owns both buffers. No callbacks run during capture. */
MvResult mv_process_inspection_snapshot(MvContext *ctx,
                                        MvProcessInfo *processes, size_t process_capacity,
                                        size_t *process_total, MvLineageInfo *lineage,
                                        size_t lineage_capacity, size_t *lineage_total);
/** Query or signal the creation lineage. The root may have ended. A valid
 * root with no live descendants gives an empty result. The query excludes the
 * root and uses the same type filter and buffer rules as mv_process_children.
 * The signal includes the root when it is live. IDs are local to ctx. */
MvResult mv_process_creation_descendants(MvContext *ctx, MvProcessId ancestor, uint64_t type,
                                         MvProcessInfo *out, size_t capacity, size_t *total);
MvResult mv_process_signal_creation_tree(MvContext *ctx, MvProcessId ancestor, MvSignal signal);
/** Run one logical scheduler tick. New processes first run on the next tick;
 * priorities are snapshotted at the beginning of the tick. Recursive ticks
 * return MV_BUSY. A callback error returns MV_PROCESS_ERROR after cleanup. */
MvResult mv_process_tick(MvContext *ctx);

/** Create an independent context and logical clock.
 * @param ticks_per_second Finite, positive logical tick rate in hertz.
 * @param max_steps Maximum ticks returned for one advance; must be nonzero.
 * @return Owned context, or NULL for invalid settings or allocation failure.
 * Destroy the context with mv_context_destroy. */
MvContext *mv_context_create(double ticks_per_second, unsigned max_steps);
/** Destroy from the owner outside process callbacks or destructors. Calls
 * made from those callbacks are ignored; destroy again after the tick. */
void mv_context_destroy(MvContext *ctx);

/** Advance the logical clock without reading the system clock.
 * @param ctx Context created by mv_context_create; must not be NULL.
 * @param elapsed_seconds Finite, nonnegative duration in seconds.
 * @param steps Receives the number of ticks; must not be NULL.
 * @return MV_OK, MV_INVALID_ARGUMENT or MV_OVERFLOW. On failure the context
 * and steps are unchanged. Time above max_steps is recorded as dropped. */
MvResult mv_context_advance(MvContext *ctx, double elapsed_seconds, unsigned *steps);
uint64_t mv_context_ticks(const MvContext *ctx);
double mv_context_dropped_seconds(const MvContext *ctx);

#ifdef __cplusplus
}
#endif
#endif
