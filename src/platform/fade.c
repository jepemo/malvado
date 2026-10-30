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

#include "malvado/platform.h"

int
mv_fade_start(MvFade *fade, MvColor target, unsigned ticks)
{
    if (!fade || ticks > 1000000)
        return 0;
    fade->start = fade->current;
    fade->target = target;
    fade->duration = ticks;
    fade->elapsed = 0;
    if (ticks == 0)
        fade->current = target;
    return 1;
}

int
mv_fade_active(const MvFade *fade)
{
    return fade && fade->elapsed < fade->duration;
}

static unsigned char
interpolate(unsigned char start, unsigned char target, unsigned elapsed, unsigned duration)
{
    int64_t difference = (int64_t)target - start;
    return (unsigned char)((int64_t)start + difference * elapsed / duration);
}

int
mv_fade_step(MvFade *fade)
{
    if (!mv_fade_active(fade))
        return 0;
    ++fade->elapsed;
    fade->current = (MvColor){
        interpolate(fade->start.r, fade->target.r, fade->elapsed, fade->duration),
        interpolate(fade->start.g, fade->target.g, fade->elapsed, fade->duration),
        interpolate(fade->start.b, fade->target.b, fade->elapsed, fade->duration),
        interpolate(fade->start.a, fade->target.a, fade->elapsed, fade->duration)
    };
    return 1;
}
