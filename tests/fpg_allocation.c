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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "malvado/fpg.h"

static size_t calls, fail_at, outstanding;

void *test_fpg_malloc(size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = malloc(size);
    if (result) ++outstanding;
    return result;
}
void *test_fpg_calloc(size_t count, size_t size)
{
    if (++calls == fail_at) return NULL;
    void *result = calloc(count, size);
    if (result) ++outstanding;
    return result;
}
void test_fpg_free(void *pointer)
{
    if (pointer) { assert(outstanding); --outstanding; }
    free(pointer);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    unsigned char data[2048];
    size_t length = fread(data, 1, sizeof(data), file);
    assert(!ferror(file) && feof(file));
    fclose(file);
    size_t failures = 0;
    for (size_t i = 1; i < 20; ++i) {
        calls = 0;
        fail_at = i;
        const char *error = NULL;
        MvFpg *fpg = mv_fpg_parse(data, length, &error);
        if (fpg) {
            assert(!error && i > calls);
            mv_fpg_destroy(fpg);
            assert(outstanding == 0);
            break;
        }
        assert(error && outstanding == 0);
        ++failures;
    }
    assert(failures > 0 && failures < 19);
    return 0;
}
