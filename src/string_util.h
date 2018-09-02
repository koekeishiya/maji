#ifndef MAJI_STRING_UTIL_H
#define MAJI_STRING_UTIL_H

#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define u8 (uint8_t*)

struct interned_string
{
    size_t length;
    uint8_t *string;
};

static struct interned_string *interned_strings;

static inline size_t
length_of_string(const uint8_t *a)
{
    size_t length = strlen((char*)a);
    return length;
}

static inline uint8_t *
copy_string_count(const uint8_t *a, size_t length)
{
    uint8_t *result = malloc(length + 1);
    memcpy(result, a, length);
    result[length] = '\0';
    return result;
}

static inline uint8_t *
copy_string(const uint8_t *a)
{
    size_t length = length_of_string(a);
    uint8_t *result = copy_string_count(a, length);
    return result;
}

static inline bool
same_string(const uint8_t *a, const uint8_t *b)
{
    bool result = a && b && (strcmp((char*)a, (char*)b) == 0);
    return result;
}

static inline uint8_t *
intern_string_count(const uint8_t *string, size_t length)
{
    for (int i = 0; i < buf_len(interned_strings); ++i) {
        if ((interned_strings[i].length == length) &&
            (strncmp((char*)interned_strings[i].string, (char*)string, length) == 0)) {
            return interned_strings[i].string;
        }
    }

    uint8_t *i_string = copy_string_count(string, length);
    buf_push(interned_strings, ((struct interned_string) {
        .length = length,
        .string = i_string
    }));
    return i_string;
}

static inline uint8_t *
intern_string(const uint8_t *a)
{
    size_t length = length_of_string(a);
    uint8_t *result = intern_string_count(a, length);
    return result;
}

static const int int_char_table[] =
{
    ['0'] = 0x0, ['1'] = 0x1,
    ['2'] = 0x2, ['3'] = 0x3,
    ['4'] = 0x4, ['5'] = 0x5,
    ['6'] = 0x6, ['7'] = 0x7,
    ['8'] = 0x8, ['9'] = 0x9,
    ['a'] = 0xA, ['A'] = 0xA,
    ['b'] = 0xB, ['B'] = 0xB,
    ['c'] = 0xC, ['C'] = 0xC,
    ['d'] = 0xD, ['D'] = 0xD,
    ['e'] = 0xE, ['E'] = 0xE,
    ['f'] = 0xF, ['F'] = 0xF,
};

static inline int
convert_string_count_to_int(const uint8_t *a, size_t length, int base)
{
    int result = 0;
    const uint8_t *e = a + length;
    for (const uint8_t *s = a; s < e; ++s) {
        result += int_char_table[*s] * (pow(base, --length));
    }
    return result;
}

static inline int
convert_string_to_int(const uint8_t *a, int base)
{
    size_t length = length_of_string(a);
    int value = convert_string_count_to_int(a, length, base);
    return value;
}

#endif
