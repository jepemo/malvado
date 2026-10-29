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

typedef enum {
    MV_OK = 0,
    MV_INVALID_ARGUMENT,
    MV_OVERFLOW,
    MV_NOT_FOUND,
    MV_OUT_OF_MEMORY,
    MV_BUSY,
    MV_PROCESS_ERROR,
    MV_QUEUE_FULL,
    MV_QUEUE_EMPTY
} MvResult;

typedef uint64_t MvProcessId;
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
} MvProcessInfo;

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

/** FIFO queue with at most 1024 messages per process. Bytes are copied on send;
 * opaque values transfer ownership only on successful send. Receive transfers
 * ownership of bytes or opaque values to the caller, who must call release. */
MvResult mv_process_send(MvContext *ctx, MvProcessId id, const MvMessage *message);
MvResult mv_process_receive(MvContext *ctx, MvProcessId id, MvMessage *message);
MvResult mv_process_message_count(MvContext *ctx, MvProcessId id, size_t *count);
void mv_message_release(MvMessage *message);

/** A successful spawn transfers ownership of user to the context. The optional
 * destructor is called once on termination or context destruction. Failed spawn
 * leaves ownership with the caller. parent=0 creates a root process. */
MvResult mv_process_spawn(MvContext *ctx, MvProcessId parent, int64_t priority,
                          MvProcessStep step, MvProcessDestroy destroy, void *user,
                          MvProcessId *id);
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
