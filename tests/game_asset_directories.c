/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#ifdef NDEBUG
#undef NDEBUG
#endif
#include "malvado/malvado.h"
#include <assert.h>
#include <string.h>

int
main(int argc, char **argv)
{
    assert(argc == 2);
    MvContext *a = mv_context_create_with_game_dir(60, 5, argv[1]);
    MvContext *b = mv_context_create_with_game_dir(60, 5, argv[1]);
    assert(a && b);
    assert(!strcmp(mv_get_assets_dir(a), argv[1]));
    assert(!strcmp(mv_context_game_dir(a), argv[1]));

    assert(mv_set_assets_dir(a, "first") == MV_OK);
    assert(mv_set_assets_dir(a, "another dir") == MV_OK);
    assert(!strcmp(mv_get_assets_dir(b), argv[1]));
    assert(mv_set_assets_dir(a, "") == MV_INVALID_ARGUMENT);

    char path[8192];
    size_t required;
    assert(mv_resolve_asset_path(a, "link/../sheet.json", NULL, 0, &required) == MV_OK);
    strcpy(path, "unchanged");
    assert(mv_resolve_asset_path(a, "link/../sheet.json", path, 2, NULL) == MV_BUFFER_TOO_SMALL);
    assert(!strcmp(path, "unchanged"));
    assert(mv_resolve_asset_path(a, "link/../sheet.json", path, sizeof(path), NULL) == MV_OK);
    assert(required == strlen(path) + 1);
    assert(!strcmp(path + strlen(argv[1]), "/another dir/link/../sheet.json"));

    const char *absolute[] = {
#ifdef _WIN32
        "C:/game assets/sheet.json", "C:\\game assets\\sheet.json",
        "\\\\server\\share\\sheet.json",
#else
        "/game assets/sheet.json", "/unix/\\file",
#endif
    };
    for (size_t i = 0; i < sizeof(absolute) / sizeof(absolute[0]); ++i) {
        assert(mv_resolve_asset_path(a, absolute[i], path, sizeof(path), NULL) == MV_OK);
        assert(!strcmp(path, absolute[i]));
    }
#ifdef _WIN32
    assert(mv_set_assets_dir(a, "C:relative") == MV_INVALID_ARGUMENT);
    assert(mv_set_assets_dir(a, "//server") == MV_INVALID_ARGUMENT);
    assert(mv_set_assets_dir(a, "/current-drive") == MV_INVALID_ARGUMENT);
    assert(mv_set_assets_dir(a, "\\current-drive") == MV_INVALID_ARGUMENT);
#endif
    assert(mv_set_assets_dir(a, mv_get_assets_dir(a)) == MV_OK);
    assert(mv_set_assets_dir(a, NULL) == MV_OK);
    assert(!strcmp(mv_get_assets_dir(a), argv[1]));
    assert(mv_set_assets_dir(b, ".") == MV_OK);
    mv_context_destroy(a);
    mv_context_destroy(b);
    return 0;
}
