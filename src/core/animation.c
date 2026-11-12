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

#include "malvado/animation.h"
#include <stdlib.h>
#include <string.h>

struct MvAnimation {
    size_t count, references;
    uint64_t duration_us;
    int loop;
    MvAnimationFrame frames[];
};

MvAnimation *
mv_animation_create(const MvAnimationFrame *frames, size_t count, int loop)
{
    if (!frames || !count || count > 8192 || (loop != 0 && loop != 1)) return NULL;
    uint64_t total = 0;
    for (size_t i = 0; i < count; ++i) {
        if (frames[i].graph_id <= 0 || !frames[i].duration_us || frames[i].duration_us > 3600000000ULL)
            return NULL;
        total += frames[i].duration_us;
    }
    MvAnimation *animation = malloc(sizeof(*animation) + count * sizeof(*frames));
    if (!animation) return NULL;
    animation->count = count;
    animation->references = 1;
    animation->duration_us = total;
    animation->loop = loop;
    memcpy(animation->frames, frames, count * sizeof(*frames));
    return animation;
}

int
mv_animation_retain(MvAnimation *animation)
{
    if (!animation || animation->references == SIZE_MAX) return 0;
    ++animation->references;
    return 1;
}

void
mv_animation_destroy(MvAnimation *animation)
{
    if (animation && --animation->references == 0) free(animation);
}

int
mv_animation_sample(const MvAnimation *animation, uint64_t elapsed_us, MvAnimationSample *out)
{
    if (!animation || !out) return 0;
    if (animation->loop) elapsed_us %= animation->duration_us;
    else if (elapsed_us >= animation->duration_us) {
        *out = (MvAnimationSample){animation->count - 1, animation->frames[animation->count - 1].graph_id};
        return 1;
    }
    for (size_t i = 0; i < animation->count; ++i) {
        if (elapsed_us < animation->frames[i].duration_us) {
            *out = (MvAnimationSample){i, animation->frames[i].graph_id};
            return 1;
        }
        elapsed_us -= animation->frames[i].duration_us;
    }
    return 0;
}

int
mv_animation_advance(const MvAnimation *animation, MvAnimationState *state, unsigned tick_rate)
{
    if (!animation || !state || !tick_rate || tick_rate > 1000000 ||
        (state->tick_rate && state->tick_rate != tick_rate) || state->remainder >= tick_rate)
        return 0;
    MvAnimationState next = *state;
    next.tick_rate = tick_rate;
    unsigned fraction = next.remainder + 1000000 % tick_rate;
    uint64_t step = 1000000 / tick_rate + fraction / tick_rate;
    next.remainder = fraction % tick_rate;
    if (animation->loop)
        next.elapsed_us = (next.elapsed_us % animation->duration_us + step) % animation->duration_us;
    else if (next.elapsed_us >= animation->duration_us || step >= animation->duration_us - next.elapsed_us)
        next.elapsed_us = animation->duration_us;
    else next.elapsed_us += step;
    *state = next;
    return 1;
}
