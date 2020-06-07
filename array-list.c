/*
 * Copyright (c) 2020 Patrick Pelletier
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "whence.h"

#include <stdlib.h>
#include <string.h>

#define MIN_CAP 5

void AL_init (ArrayList *al) {
    memset (al, 0, sizeof (*al));
}

void AL_add (ArrayList *al, const char *str) {
    char *newStr = MY_STRDUP (str);
    AL_add_nocopy (al, newStr);
}

void AL_add_nocopy (ArrayList *al, char *str) {
    if (al->size >= al->capacity) {
        size_t newCap = al->capacity * 2;
        if (newCap < MIN_CAP) {
            newCap = MIN_CAP;
        }
        al->strings = realloc (al->strings, newCap * sizeof (char *));
        CHECK_NULL (al->strings);
	al->capacity = newCap;
    }

    al->strings[al->size++] = str;
}

void AL_clear (ArrayList *al) {
    size_t i;

    for (i = 0; i < al->size; i++) {
        free (al->strings[i]);
    }

    al->size = 0;
}

void AL_cleanup (ArrayList *al) {
    AL_clear (al);
    free (al->strings);
    AL_init (al);
}

char *AL_join (const ArrayList *al) {
    size_t len = 1;             /* for NUL terminator */
    size_t i;

    for (i = 0; i < al->size; i++) {
        len += strlen (al->strings[i]);
    }

    char *s = malloc (len);
    CHECK_NULL (s);

    len = 0;
    for (i = 0; i < al->size; i++) {
        const size_t len1 = strlen (al->strings[i]);
        memcpy (s + len, al->strings[i], len1);
        len += len1;
    }

    s[len] = 0;                 /* NUL terminate */
    return s;
}
