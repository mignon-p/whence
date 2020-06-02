#include "whence.h"

#include <stdlib.h>
#include <string.h>

void AL_init (ArrayList *al) {
    memset (al, 0, sizeof (*al));
}

#define MIN_CAP 5

void AL_add (ArrayList *al, const char *str) {
    if (al->size <= al->capacity) {
        size_t newCap = al->capacity * 2;
        if (newCap < MIN_CAP) {
            newCap = MIN_CAP;
        }
        al->strings = realloc (al->strings, newCap * sizeof (char *));
        CHECK_NULL (al->strings);
    }

    char *newStr = strdup (str);
    CHECK_NULL (newStr);
    al->strings[al->size++] = newStr;
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
