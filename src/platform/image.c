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

#include <math.h>
#include "malvado/platform.h"

int
mv_image_region_valid(const MvImage *image, MvImageRegion region, MvImageTransform transform)
{
    int width = mv_image_width(image);
    int height = mv_image_height(image);
    if (!image || region.x < 0 || region.y < 0 || region.width <= 0 || region.height <= 0 ||
        region.x >= width || region.y >= height ||
        region.width > width - region.x || region.height > height - region.y)
        return 0;
    if (!isfinite(transform.x) || !isfinite(transform.y) ||
        !isfinite(transform.pivot_x) || !isfinite(transform.pivot_y) ||
        !isfinite(transform.scale) || !isfinite(transform.rotation) || transform.scale <= 0)
        return 0;
    return isfinite(region.width * transform.scale) &&
           isfinite(region.height * transform.scale) &&
           isfinite(transform.pivot_x * transform.scale) &&
           isfinite(transform.pivot_y * transform.scale);
}
