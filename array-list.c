#include "whence.h"

#include <stdlib.h>
#include <string.h>

#define MIN_CAP 5

void AL_init (ArrayList *al) {
    memset (al, 0, sizeof (*al));
}

void AL_add (ArrayList *al, const char *str) {
    char *newStr = strdup (str);
    CHECK_NULL (newStr);
    AL_add_nocopy (al, newStr);
}

void AL_add_nocopy (ArrayList *al, char *str) {
    if (al->size <= al->capacity) {
        size_t newCap = al->capacity * 2;
        if (newCap < MIN_CAP) {
            newCap = MIN_CAP;
        }
        al->strings = realloc (al->strings, newCap * sizeof (char *));
        CHECK_NULL (al->strings);
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
