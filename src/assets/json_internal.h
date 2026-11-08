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

#ifndef MALVADO_JSON_INTERNAL_H
#define MALVADO_JSON_INTERNAL_H

#include <math.h>
#include <string.h>
#include "cJSON.h"

static int
json_numbers_valid(const char *text, size_t length)
{
    size_t i = 0;
    while (i < length) {
        if (text[i] == '"') {
            ++i;
            while (i < length && text[i] != '"') {
                if ((unsigned char)text[i] < 32)
                    return 0;
                if (text[i] == '\\' && i + 1 < length) {
                    if (text[i + 1] == 'u' && length - i >= 6 &&
                        !memcmp(text + i + 2, "0000", 4))
                        return 0;
                    ++i;
                }
                ++i;
            }
            if (i < length)
                ++i;
            continue;
        }
        unsigned char byte = (unsigned char)text[i];
        if (byte < 32 && byte != '\t' && byte != '\n' && byte != '\r')
            return 0;
        if (text[i] != '-' && (text[i] < '0' || text[i] > '9')) {
            ++i;
            continue;
        }
        if (text[i] == '-')
            ++i;
        if (i == length || text[i] < '0' || text[i] > '9')
            return 0;
        if (text[i++] == '0') {
            if (i < length && text[i] >= '0' && text[i] <= '9')
                return 0;
        } else {
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && text[i] == '.') {
            ++i;
            if (i == length || text[i] < '0' || text[i] > '9')
                return 0;
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && (text[i] == 'e' || text[i] == 'E')) {
            ++i;
            if (i < length && (text[i] == '+' || text[i] == '-'))
                ++i;
            if (i == length || text[i] < '0' || text[i] > '9')
                return 0;
            while (i < length && text[i] >= '0' && text[i] <= '9')
                ++i;
        }
        if (i < length && text[i] != ',' && text[i] != ']' && text[i] != '}' &&
            text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
            return 0;
    }
    return 1;
}

static int
object_fields(const cJSON *object, const char *const *allowed, size_t count)
{
    if (!cJSON_IsObject(object))
        return 0;
    for (const cJSON *field = object->child; field; field = field->next) {
        size_t i;
        for (i = 0; i < count && strcmp(field->string, allowed[i]) != 0; ++i) {}
        if (i == count)
            return 0;
        for (const cJSON *other = field->next; other; other = other->next) {
            if (strcmp(field->string, other->string) == 0)
                return 0;
        }
    }
    return 1;
}

static int
integer_field(const cJSON *object, const char *name, int minimum, int maximum, int *value)
{
    const cJSON *field = cJSON_GetObjectItemCaseSensitive(object, name);
    if (!cJSON_IsNumber(field) || !isfinite(field->valuedouble) ||
        field->valuedouble < minimum || field->valuedouble > maximum ||
        floor(field->valuedouble) != field->valuedouble)
        return 0;
    *value = (int)field->valuedouble;
    return 1;
}

#endif
