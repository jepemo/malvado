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

#ifdef MALVADO_INSTALLED_VERSION_TEST
#include "expected-version.h"
#endif
#include "malvado/malvado.h"
#include <assert.h>
#include <string.h>

int
main(void)
{
    assert(mv_abi_version() == MV_ABI_VERSION);
    assert(strcmp(mv_version_string(), MALVADO_VERSION_STRING) == 0);
    const char *version = mv_version_string();
    MvContext *ctx = mv_context_create(60, 5);
    assert(ctx);
    mv_context_destroy(ctx);
    assert(mv_version_string() == version);
    assert(mv_abi_version() == MV_ABI_VERSION);
    return 0;
}
